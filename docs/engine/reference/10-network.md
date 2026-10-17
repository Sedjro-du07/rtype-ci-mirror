# 10 · Network (engine side)

Status: **Locked**. ADRs 0011, 0033, 0037 (players, typed messages, transports), 0041–0043 (protocol v1). Resolves E-03 (entity identity on the wire).
Folder: `engine/include/engine/net/`, `engine/src/net/`, `engine/plugins/network_asio/`.

## Role

Everything an online Pong would also need (ADR 0011): transport, connections, heartbeats and
timeouts, a network thread, packet bounds, replication of components, sending player actions.
**This block fixes classes, threads and responsibilities. The bytes (header layout, message ids,
reliability scheme, byte order) are defined by the protocol RFC, [`docs/network/rfc.md`](../../network/rfc.md)
(MSEP v1.0); section numbers below (RFC §n) point into it.**

What the game sees (ADR 0037): it opens the server or connects explicitly, deals with **players**
(`PlayerId`) and **typed messages**. Sockets, threads, connections, bytes and message ids stay
inside the engine.

---

## Basic types — `net/NetTypes.hpp`

```cpp
using PlayerId     = uint32_t;                         // what the game sees; 0 = none; one per connected player
using ConnectionId = uint32_t;                         // engine-internal: one UDP endpoint and its TCP stream
struct Endpoint { std::array<uint8_t, 16> address{}; uint16_t port = 0; bool v6 = false;
                  std::string toString() const; static Result<Endpoint> resolve(std::string_view host, uint16_t port); };
struct Datagram { Endpoint from; std::vector<std::byte> bytes; };
// Transport (Unreliable, Reliable, Stream) is declared in reflection/TypeInfo.hpp and chosen per message.
enum class DisconnectReason : uint8_t { Quit, Timeout, Kicked, ProtocolError, ServerShutdown,
                                        ServerFull, FormatMismatch, ProtocolMismatch, WrongGame,
                                        InvalidName, RefusedByGame, ConnectionLost };   // codes: RFC §6.8

struct NetConfig {
    uint16_t port = 4242;
    uint32_t maxClients = 32;             // whole server, lobby included: rooms.maxRooms (8) × 4 players
    double   heartbeatSeconds = 0.5;
    double   timeoutSeconds = 5.0;        // no packet for this long → PlayerDisconnected{Timeout} (grace) / ConnectionInterrupted
    uint32_t maxDatagramBytes = 1200;     // larger datagrams are dropped, never allocated
    uint32_t maxQueuedDatagrams = 1024;   // inbox bound per frame; excess is dropped and counted
    double   snapshotRate = 20.0;         // replication snapshots per second
    uint32_t maxNameLength = 16;          // player names longer than this are refused at handshake
    uint32_t maxStreamMessageBytes = 65536; // TCP: a larger announced message closes the stream
    double   handshakeSeconds = 5.0;      // TCP connect → Accepted, else Refused(Timeout) (RFC §6.2)
    uint32_t maxPendingHandshakes = 16;   // and 4 per address (RFC §11)
    double   reconnectGraceSeconds = 30.0;// lost session kept this long for its token; 0 = none (RFC §6.7)
    bool     autoReconnect = true;        // client: retry every 2 s during the grace period
    uint32_t sendBudgetBytesPerSecond = 65536; // snapshot bytes per player (RFC §9.6)
    double   interpolationDelay = 0.1;    // client: other entities drawn this far behind (RFC §9.5)
    uint32_t inputRedundancy = 3;         // copies of each input, 1 to 4 (RFC §9.4)
    bool     shareOnlinePlayers = true;   // send the online players list to lobby clients (RFC §8.1.8)
};

struct ConnectionStats { double rttMs; double lossPercent; uint64_t bytesIn, bytesOut;
                         uint64_t packetsIn, packetsOut, dropped; };
```

Also: `Endpoint::ipv4(a, b, c, d, port)`, `operator==` and `std::hash<Endpoint>`; `toString` prints
`1.2.3.4:5` and `[2001:db8::1]:80` (longest zero run compressed); `resolve` returns the first address of
`os::resolveHost`. `DisconnectReason` is registered with `ENGINE_ENUM`.

## `class INetwork` — `net/INetwork.hpp` (transport plugin, engine only)

Runs its own I/O thread. Only moves bytes; knows nothing about connections or messages.

```cpp
class INetwork : public IPlugin {
public:
    virtual Result<void> openUdp(uint16_t port) = 0;          // server: bind port; client: port 0
    virtual void         closeUdp() = 0;
    virtual void         sendUdp(const Endpoint& to, std::span<const std::byte> bytes) = 0; // thread-safe, copies
    virtual void         setReceiver(std::move_only_function<void(Datagram&&)> onDatagram) = 0; // called on the I/O thread

    // TCP, for Stream messages (lobby, room lists, text chat; ADR 0037). Same shape, stream-based.
    virtual Result<void> listenTcp(uint16_t port) = 0;
    virtual Result<uint32_t> connectTcp(const Endpoint& to) = 0;
    virtual void         sendTcp(uint32_t stream, std::span<const std::byte> bytes) = 0;
    virtual void         setTcpReceiver(std::move_only_function<void(uint32_t stream, std::span<const std::byte>)>) = 0;
    virtual void         closeTcp(uint32_t stream) = 0;
};
```

## `class NetworkSession` — `net/NetworkSession.hpp` (core)

Owns connections and players. The I/O thread pushes datagrams and stream data into a bounded,
mutex-protected inbox; the loop thread drains it in `poll()`. All public methods are loop-thread only.

```cpp
class NetworkSession {
public:
    NetworkSession(EventBus& events, Logger& log, const NetConfig& config, const TypeRegistry& types);
    void attach(INetwork& transport, NetworkIdMap& ids);   // engine only, after the plugin is loaded

    // ---- Game API: opening and closing (ADR 0037, explicit) ----
    Result<void> startServer(std::optional<uint16_t> port = {}); // server: open UDP + TCP on port
                                                           // (default: NetConfig::port, i.e. --port / config)
    void stopServer();                                     // tells every player (ServerShutdown), closes
    Result<void> connectTo(std::string_view host, uint16_t port, std::string_view playerName); // client
    template <class T> Result<void> connectTo(std::string_view host, uint16_t port,
                                              std::string_view playerName, const T& admissionData); // T: ENGINE_MESSAGE (ADR 0042)
    void disconnect();                                     // client: leave (Quit)

    // ---- Game API: players ----
    std::span<const PlayerId> players() const;             // server: connected players
    PlayerId         localPlayer() const;                  // client: my id (0 before JoinedServer)
    std::string_view name(PlayerId p) const;               // name given to connectTo
    void             kick(PlayerId p, std::string_view reason); // server; reason reaches LeftServer::message
    template <class T> void admission(std::move_only_function<Admission(const AdmissionRequest&, const T&)> fn);
                                                           // server (lobby code): accept or refuse joining players (ADR 0042)
    double           ping(PlayerId p) const;               // round-trip time in ms
    ConnectionStats  stats(PlayerId p) const;              // lagometer, console "net"
    bool isServer() const; bool connected() const;

    // ---- Game API: typed messages (T registered with ENGINE_MESSAGE) ----
    template <class T> void send(PlayerId to, const T& msg);           // server → one player
    template <class T> void send(const T& msg);                        // client → server
    template <class T> void broadcast(const T& msg, std::optional<PlayerId> except = {}); // server → all
    template <class T> [[nodiscard]] Subscription on(
        std::move_only_function<void(Context&, PlayerId from, const T&)> fn);   // server (and client)
    template <class T> [[nodiscard]] Subscription on(
        std::move_only_function<void(Context&, const T&)> fn);                  // client: from = server

    // ---- Engine only ----
    void poll(Context& ctx);   // drain inbox → validate size / header → route to connections →
                               // handshake (name, format fingerprint), heartbeats, timeouts →
                               // decode messages → call on<T> callbacks → publish player events
    void sendRaw(ConnectionId to, Transport t, uint16_t messageId, std::span<const std::byte> payload);
};

struct AdmissionRequest { std::string_view name; Endpoint from; bool reconnecting; };
class Admission {                                          // returned by the admission callback
public:
    static Admission accept();
    static Admission refuse(std::string reason);           // ≤ 128 bytes, shown to the player
    bool accepted() const; std::string_view reason() const;
};
```

### Messages

- `send` / `broadcast` look up the `TypeInfo` of `T`, encode it with the generic codec (block 4) into
  a bounded `ByteWriter`, and send it on the transport declared on the message: `unreliable()` → UDP,
  `reliable()` (default) → UDP with acknowledgements, `stream()` → TCP.
- The message id comes from the position of `T` in `TypeRegistry::messages()`; client and server load
  the same game module, so they agree. Its width and encoding are defined by the protocol RFC.
- `Entity` fields are written as `NetworkId` and read back through `NetworkIdMap`; an entity unknown
  on arrival becomes `Entity::null()` (for example a message that arrives before the snapshot that
  spawns the entity). Game callbacks must check `isNull()`.
- `on<T>` callbacks run **on the loop thread during `poll()`**, at the start of the frame, before
  `FixedUpdate`, in arrival order. Structural changes go through `ctx.commands()`. An exception in a
  callback is caught and logged with the message name; the message is ignored; the session continues.
- A message that fails to decode (missing bytes, limit exceeded, unknown enum value) is dropped and
  counted; it never reaches game code.

### Players and the handshake

- A client's TCP stream and UDP endpoint are linked to one `PlayerId` (how is defined by the protocol
  RFC). One connection is one player in Part 1.
- The handshake carries the player name and `TypeRegistry::formatFingerprint()`. A server refuses a
  client whose fingerprint differs (`FormatMismatch`), whose name is empty or too long, or when
  `maxClients` players are connected to the whole server, lobby included (`ServerFull`); the client receives `LeftServer{reason}` with a log line
  explaining what to rebuild or change.
- Admission (ADR 0042): after the engine's checks (protocol version, format fingerprint), the game
  accepts or refuses the player with its own reason, from the name and data of the game carried by the
  Hello. The extension point is defined with the protocol RFC.
- Events published (block 6): `PlayerJoined{player}`, `PlayerLeft{player, reason, message}`,
  `PlayerDisconnected{player, reason}`, `PlayerReconnected{player}` on the server; `JoinedServer{me}`,
  `LeftServer{reason, message}`, `ConnectionInterrupted{reason}`, `Reconnected{}` on the client.
- Order of the server's checks, admission and reconnection: RFC §6.4 and §6.7 (ADR 0042, 0043).

Every datagram is checked against `maxDatagramBytes`, the protocol magic/version and the header
bounds before any handler runs; every stream message against `maxStreamMessageBytes`. A malformed
input is dropped and counted; it never throws, allocates beyond the limit, or reaches game code.

### What ENG-38 provides (the core)

```cpp
struct NetCounters { uint64_t droppedSize, droppedMagic, droppedKind, droppedSession, droppedSequence,
                              droppedMessage, droppedRate, droppedInbox, closedProtocol; };   // RFC §11
struct InboundMessage { ConnectionId from; uint16_t id; std::optional<uint16_t> messageSeq;
                        std::span<const std::byte> payload; };            // valid during the callback

    void attach(INetwork& transport);                      // installs the bounded inbox as receiver
    Result<void> open(bool server, uint16_t port);         // openUdp(port) or openUdp(0)
    void close();                                          // closes every connection, then UDP
    ConnectionId openConnection(const Endpoint& udp, uint32_t sessionKey, PlayerId player, uint32_t stream = 0);
    void closeConnection(ConnectionId, DisconnectReason, std::string message = {}); // PlayerLeft / LeftServer
    void onMessage(fn(const InboundMessage&));             // SRV-03 plugs the dispatcher here
    void onUdpLink(fn(const Endpoint&, uint64_t token));   // SRV-02 binds the address
    void poll(double now);                                 // inbox → RFC §4.6 checks 1–6 → acks → messages;
                                                           // then timeouts
    void sendRaw(ConnectionId, Transport, uint16_t id, std::span<const std::byte> payload);
                                                           // Stream: one TCP frame now; UDP: queued
    void flush(double now);                                // packs the queued UDP messages into Data packets
                                                           // (≤ maxDatagramBytes), or a heartbeat after 0.5 s
    std::vector<ConnectionId> connections() const; ConnectionStats stats(ConnectionId) const;
    const NetCounters& counters() const;
```

Transport of an id: game ids `0x0100 + i` → `TypeRegistry::messages()[i]->transport`; engine ids from
the table of RFC §8.5; anything else is invalid. RTT `= 0.9 × rtt + 0.1 × sample` from acknowledged
packets; loss = share of the last 64 sent packets not acknowledged after 1 s. More than 50 invalid
packets in 10 s close the connection (`ProtocolError`); more than 300 packets in a second are dropped.

Until the server tasks build on it: `poll` takes the time instead of `Context` (messages reach
`onMessage`, not `on<T>`, SRV-03); a timeout publishes `PlayerLeft{Timeout}` / `LeftServer{Timeout}`
(the grace period with `PlayerDisconnected` comes with SRV-16); reliable messages get their
`messageSeq` but are sent once (resends with SRV-05); `attach` has no `NetworkIdMap` yet (ENG-39);
`startServer`, `connectTo`, players and typed messages are SRV-01 to SRV-03.

## Internal structure (engine only) — `engine/src/net/`

`NetworkSession` is a facade over small private classes. Their headers live next to their sources
in `engine/src/net/`, not under `include/`, so games cannot include them. Tasks: server board.

| Class | File | Task | Responsibility |
|---|---|---|---|
| `PlayerTable` | `src/net/PlayerTable.hpp` | SRV-01 | `PlayerId` ↔ `ConnectionId`, names, allocation and release of ids |
| `Handshake` | `src/net/Handshake.hpp` | SRV-02 | connection state machine on both sides: name, format fingerprint, refusals, TCP ↔ UDP link |
| `MessageDispatcher` | `src/net/MessageDispatcher.hpp` | SRV-03 | message id ↔ `TypeInfo`, `on<T>` callback lists, safe invocation (exceptions caught) |
| `ReliableChannel` | `src/net/ReliableChannel.hpp` | SRV-05 | per connection: sequence numbers, acknowledgements, resends, duplicate drop for `reliable()` |
| `StreamFramer` | `src/net/StreamFramer.hpp` | SRV-06 | cuts a TCP byte stream into messages; refuses announced sizes over `maxStreamMessageBytes` |
| `Codec` | `src/reflection/Codec.hpp` | SRV-04 | generic `encode` / `decode` of any registered type (lists, structs, enums, `Entity` through `WireContext`) |

```cpp
class PlayerTable {
public:
    PlayerId add(ConnectionId c, std::string name);       // next free id, never 0
    void     remove(PlayerId p);
    PlayerId playerOf(ConnectionId c) const;              // 0 if unknown
    ConnectionId connectionOf(PlayerId p) const;
    std::string_view name(PlayerId p) const;
    std::span<const PlayerId> players() const;
};

class MessageDispatcher {
public:
    explicit MessageDispatcher(const TypeRegistry& types, Logger& log);
    uint16_t idOf(uint64_t typeKey) const;                // position in TypeRegistry::messages()
    const TypeInfo* typeOf(uint16_t id) const;            // nullptr → message dropped and counted
    Subscription add(uint64_t typeKey, std::move_only_function<void(Context&, PlayerId, const void*)> fn);
    void dispatch(Context& ctx, PlayerId from, uint16_t id, ByteReader& payload, const WireContext& wire);
};

class ReliableChannel {                                   // RFC §7: packet acks + reliable ordered messages
public:
    uint16_t nextSequence();                              // for an outgoing Data packet
    void     queue(uint16_t messageId, std::span<const std::byte> payload);   // reliable message
    void     fill(uint16_t sequence, ByteWriter& out, double now); // writes due and new reliable messages
    void     writeAcks(ByteWriter& out) const;            // ack, ackBits of the header
    bool     onPacket(uint16_t sequence, uint16_t ack, uint32_t ackBits, double now); // false: duplicate / too old
    void     onReliable(uint16_t messageSeq, uint16_t messageId, std::vector<std::byte> payload);
    std::vector<Delivered> deliverable();                 // in messageSeq order
    std::vector<uint16_t>  newlyAcked();                  // packet sequences, for replication (RFC §9.3)
    double   rttMs() const; double lossPercent() const;
};

class StreamFramer {
public:
    explicit StreamFramer(uint32_t maxMessageBytes);
    Result<void> feed(std::span<const std::byte> bytes);  // error → close the stream
    std::optional<std::vector<std::byte>> next();         // one complete message, or nothing yet
};
```

`Handshake` follows RFC §6 (messages, order of checks, admission, reconnection); `Codec` follows RFC
§5 (field encoding) and block 4 (`TypeInfo`).

## Rooms

With several games on one server (ADR 0038/0039), a room sees the network through `RoomNetwork`
(reference 14): only its own players. Game messages from a player are routed to the player's room, or
to the lobby when the player is not in a game (`PlayerRouting`). Replication sends a player only the
entities of their room and resets that player's state when they enter or leave a room; each room has
its own `NetworkIdMap`.

## Replication

### Components — `net/NetComponents.hpp`

```cpp
struct NetworkId  { uint32_t value = 0; };        // shared identity; never reused during a session
struct NetPrefab  { uint32_t prefabKey = 0; };    // fnv1a of the prefab path, to spawn on clients
struct NetOwner   { PlayerId owner = 0; };        // which player controls this entity
struct RemoteInput{ ActionState state; uint64_t lastMask = 0; uint32_t lastTick = 0; }; // server side
struct NetInterpolation { Vec2 from, to; double fromTime = 0, toTime = 0; };              // client side
struct NetPriority { float weight = 1.0f; };       // server: share of the send budget (RFC §9.6); absent = 1
```

### `class NetworkIdMap` — `net/NetworkIdMap.hpp`

```cpp
class NetworkIdMap {
public:
    uint32_t allocate();                              // server: next id, monotonic
    void     bind(uint32_t id, Entity e);
    void     unbind(uint32_t id);
    Entity   find(uint32_t id) const;                 // client: unknown id → Entity::null(), never a crash
    void     clear();
};
```

### Engine systems (registered by the engine, block 13)

| System | Stage | Role | Does |
|---|---|---|---|
| `engine.net.assign_ids` | LateUpdate | server | gives a `NetworkId` to new entities that have `NetPrefab` |
| `engine.net.snapshot` | LateUpdate | server | at `snapshotRate`, per player: spawns, destroys and changed `Replicated` components not yet acknowledged, by priority within the budget, relevant entities only (RFC §9.3, §9.6) |
| `engine.net.apply_snapshot` | FixedUpdate (first) | client | decodes, maps ids, spawns from `NetPrefab`, writes components, fills `NetInterpolation` |
| `engine.net.interpolate` | Render (first) | client | positions networked entities `net.interpolationDelay` behind, between two snapshots; a default the game can disable to use its own (ADR 0041) |
| `engine.net.send_input` | LateUpdate | client | sends `ActionState::toMask` for the tick |
| `engine.net.receive_input` | FixedUpdate (first) | server | writes `RemoteInput` of entities owned by each player (`NetOwner`) |

Which components are `Replicated` is declared by the game (block 4 flags). Network optimizations
(ADR 0041): the engine gives the means (protocol fields such as input sequence numbers and the last
input processed, settings, extension points), and the game team writes its prediction and
reconciliation and tunes rates and precision. Settings (ADR 0043, RFC §9.6): `net.snapshotRate`,
`net.sendBudgetBytesPerSecond`, `NetPriority{weight}`, `RoomNetwork::setRelevance`, `.quantize(min, max)`,
`net.interpolationDelay`, `net.inputRedundancy`, and the `engine.net.interpolate` switch.

## Plugin

| Plugin | Implements | Notes |
|---|---|---|
| `network_asio` | `AsioNetwork : INetwork` | standalone Asio, one `io_context` on its own thread, UDP socket + optional TCP acceptor |
