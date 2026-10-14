# ENG-38 · NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **10 Network** · **Wave 5** · status and owner live on the board.

## Goal

The core of NetworkSession: connections and the safe path from the network thread to the game thread.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/net/NetworkSession.hpp` (shared with [SRV-01](SRV-01.md))
- `src/net/NetworkSession.cpp` (shared with [SRV-01](SRV-01.md))

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

> The class below is shared: ENG-38 writes the core (constructor, attach, poll, sendRaw, isServer, connected, stats); SRV-01 the player methods; SRV-03 the message methods.

### `include/engine/net/NetworkSession.hpp`

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
    void disconnect();                                     // client: leave (Quit)

    // ---- Game API: players ----
    std::span<const PlayerId> players() const;             // server: connected players
    PlayerId         localPlayer() const;                  // client: my id (0 before JoinedServer)
    std::string_view name(PlayerId p) const;               // name given to connectTo
    void             kick(PlayerId p, std::string_view reason); // server
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
```

## Depends on

- [ENG-36](ENG-36.md) — INetwork and network types
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events
- [ENG-24](ENG-24.md) — ByteWriter / ByteReader: bounds-checked, bit packing

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-39](ENG-39.md) — Replication: NetworkIdMap, net components, snapshot and input systems
- [ENG-55](ENG-55.md) — Unit tests: scheduler, events, input, scenes, session
- [SRV-01](SRV-01.md) — Players: PlayerTable, startServer / connectTo, player events, names, kick, ping
- [SRV-05](SRV-05.md) — ReliableChannel: acknowledged and resent UDP messages

## Read first

- [ADR 0011](../adr/0011-generic-server-role.md)
- [ADR 0033](../adr/0033-engine-networking.md)
- [ADR 0037](../adr/0037-game-facing-networking.md)
- [reference 10](../engine/reference/10-network.md)
- protocol RFC (Network part, not written yet)

## To implement

- [ ] Bounded, mutex-protected inbox (`maxQueuedDatagrams`)
- [ ] `poll` skeleton: drain, size and header validation, route to connections
- [ ] Connections: heartbeats, timeouts, stats (RTT, loss, bytes)
- [ ] `attach`, `sendRaw`, `isServer`, `connected`

## Done when

- Tests: malformed and oversized datagrams dropped and counted; timeout publishes the leave event

## Watch out

- Nothing from the I/O thread reaches game code directly
- Game-facing API is built on top by SRV-01 and SRV-03

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
