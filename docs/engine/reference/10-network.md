# 10 · Network (engine side)

Status: **Locked**. ADRs 0011, 0033. Resolves E-03 (entity identity on the wire).
Folder: `engine/include/engine/net/`, `engine/src/net/`, `engine/plugins/network_asio/`.

## Role

Everything an online Pong would also need (ADR 0011): transport, connections, heartbeats and
timeouts, a network thread, packet bounds, replication of components, sending player actions.
**This block fixes classes, threads and responsibilities. The bytes (header layout, message ids,
reliability scheme, byte order) are defined by the protocol RFC in the Network part.**

---

## Basic types — `net/NetTypes.hpp`

```cpp
using ConnectionId = uint32_t;                         // 0 = invalid; assigned by the server
struct Endpoint { std::array<uint8_t, 16> address{}; uint16_t port = 0; bool v6 = false;
                  std::string toString() const; static Result<Endpoint> resolve(std::string_view host, uint16_t port); };
struct Datagram { Endpoint from; std::vector<std::byte> bytes; };
enum class Channel : uint8_t { Unreliable, Reliable };  // reliable = acknowledged and resent (scheme in the RFC)
enum class DisconnectReason : uint8_t { Quit, Timeout, Kicked, ProtocolError, ServerShutdown };

struct NetConfig {
    uint16_t port = 4242;
    uint32_t maxClients = 4;
    double   heartbeatSeconds = 0.5;
    double   timeoutSeconds = 5.0;        // no packet for this long → ClientDisconnected{Timeout}
    uint32_t maxDatagramBytes = 1200;     // larger datagrams are dropped, never allocated
    uint32_t maxQueuedDatagrams = 1024;   // inbox bound per frame; excess is dropped and counted
    double   snapshotRate = 20.0;         // replication snapshots per second
};

struct ConnectionStats { double rttMs; double lossPercent; uint64_t bytesIn, bytesOut;
                         uint64_t packetsIn, packetsOut, dropped; };
```

## `class INetwork` — `net/INetwork.hpp` (transport plugin)

Runs its own I/O thread. Only moves bytes; knows nothing about connections or messages.

```cpp
class INetwork : public IPlugin {
public:
    virtual Result<void> openUdp(uint16_t port) = 0;          // server: bind port; client: port 0
    virtual void         closeUdp() = 0;
    virtual void         sendUdp(const Endpoint& to, std::span<const std::byte> bytes) = 0; // thread-safe, copies
    virtual void         setReceiver(std::move_only_function<void(Datagram&&)> onDatagram) = 0; // called on the I/O thread

    // Optional TCP (lobby, login, chat; justified in the Server part). Same shape, stream-based.
    virtual Result<void> listenTcp(uint16_t port) = 0;
    virtual Result<uint32_t> connectTcp(const Endpoint& to) = 0;
    virtual void         sendTcp(uint32_t stream, std::span<const std::byte> bytes) = 0;
    virtual void         setTcpReceiver(std::move_only_function<void(uint32_t stream, std::span<const std::byte>)>) = 0;
    virtual void         closeTcp(uint32_t stream) = 0;
};
```

## `class NetworkSession` — `net/NetworkSession.hpp` (core)

Owns connections. The I/O thread pushes datagrams into a bounded, mutex-protected inbox; the loop
thread drains it in `poll()`. All public methods are loop-thread only.

```cpp
class NetworkSession {
public:
    NetworkSession(EventBus& events, Logger& log, const NetConfig& config);
    void attach(INetwork& transport);                      // engine only, after the plugin is loaded

    Result<void> host();                                   // server: open the port, accept up to maxClients
    Result<void> connect(std::string_view host, uint16_t port); // client: start the handshake
    void disconnect(DisconnectReason reason = DisconnectReason::Quit);
    void kick(ConnectionId id, std::string_view reason);   // server

    void poll();                     // engine only: drain inbox → validate size/header → route to
                                     // connections → heartbeats, timeouts → publish connection events
    void send(ConnectionId to, Channel ch, uint16_t messageType, std::span<const std::byte> payload);
    void broadcast(Channel ch, uint16_t messageType, std::span<const std::byte> payload,
                   std::optional<ConnectionId> except = {});
    void onMessage(uint16_t messageType,
                   std::move_only_function<void(ConnectionId from, ByteReader&)> handler); // loop thread

    bool isServer() const; bool connected() const;
    ConnectionId localId() const;                          // client: id given by the server
    std::span<const ConnectionId> connections() const;
    ConnectionStats stats(ConnectionId id) const;          // lagometer, console "net"
};
```

Every datagram is checked against `maxDatagramBytes`, the protocol magic/version and the header
bounds before any handler runs. A malformed datagram is dropped and counted; it never throws,
allocates beyond the limit, or reaches game code.

## Replication

### Components — `net/NetComponents.hpp`

```cpp
struct NetworkId  { uint32_t value = 0; };        // shared identity; never reused during a session
struct NetPrefab  { uint32_t prefabKey = 0; };    // fnv1a of the prefab path, to spawn on clients
struct NetOwner   { ConnectionId owner = 0; };    // which client controls this entity
struct RemoteInput{ ActionState state; uint64_t lastMask = 0; uint32_t lastTick = 0; }; // server side
struct NetInterpolation { Vec2 from, to; double fromTime = 0, toTime = 0; };              // client side
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
| `engine.net.snapshot` | LateUpdate | server | at `snapshotRate`, encodes every `Replicated` component of networked entities through `TypeInfo::encode`, sends spawn / update / destroy |
| `engine.net.apply_snapshot` | FixedUpdate (first) | client | decodes, maps ids, spawns from `NetPrefab`, writes components, fills `NetInterpolation` |
| `engine.net.interpolate` | Render (first) | client | positions networked entities between the two last snapshots |
| `engine.net.send_input` | LateUpdate | client | sends `ActionState::toMask` for the tick |
| `engine.net.receive_input` | FixedUpdate (first) | server | writes `RemoteInput` of entities owned by each connection |

Which components are `Replicated` is declared by the game (block 4 flags). Prediction and
reconciliation, delta snapshots and compression belong to the Network part (Track 2).

## Plugin

| Plugin | Implements | Notes |
|---|---|---|
| `network_asio` | `AsioNetwork : INetwork` | standalone Asio, one `io_context` on its own thread, UDP socket + optional TCP acceptor |
