# SRV-01 · Players: PlayerTable, startServer / connectTo, player events, names, kick, ping

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **01 Players** · status and owner live on the board.

## Goal

Give the game players instead of connections, and explicit server start / client connect (ADR 0037).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/net/PlayerTable.hpp`
- `src/net/PlayerTable.cpp`
- `include/engine/net/NetworkSession.hpp` (shared with [ENG-38](ENG-38.md))
- `src/net/NetworkSession.cpp` (shared with [ENG-38](ENG-38.md))

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/net/PlayerTable.hpp`

```cpp
class PlayerTable {
public:
    PlayerId add(ConnectionId c, std::string name);       // next free id, never 0
    void     remove(PlayerId p);
    PlayerId playerOf(ConnectionId c) const;              // 0 if unknown
    ConnectionId connectionOf(PlayerId p) const;          // 0 if unknown
    std::string_view name(PlayerId p) const;
    std::span<const PlayerId> players() const;
    size_t   size() const;
private:
    std::vector<PlayerId>                       order_;
    std::unordered_map<PlayerId, ConnectionId>  toConnection_;
    std::unordered_map<ConnectionId, PlayerId>  toPlayer_;
    std::unordered_map<PlayerId, std::string>   names_;
    PlayerId                                    next_ = 1;
};
```

### `include/engine/net/NetworkSession.hpp`

```cpp
// The player part of NetworkSession (core: ENG-38, messages: SRV-03).
class NetworkSession {
public:
    Result<void> startServer(std::optional<uint16_t> port = {});
    void stopServer();
    Result<void> connectTo(std::string_view host, uint16_t port, std::string_view playerName);
    void disconnect();
    std::span<const PlayerId> players() const;
    PlayerId         localPlayer() const;
    std::string_view name(PlayerId p) const;
    void             kick(PlayerId p, std::string_view reason);
    double           ping(PlayerId p) const;
    ConnectionStats  stats(PlayerId p) const;
private:
    PlayerTable players_;
    PlayerId    localPlayer_ = 0;
    void onPlayerAccepted(ConnectionId c, std::string name);       // publishes PlayerJoined / JoinedServer
    void onConnectionClosed(ConnectionId c, DisconnectReason r);   // publishes PlayerLeft / LeftServer
};
```

## Depends on

- [ENG-38](ENG-38.md) — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts

## Unlocks

- [SRV-02](SRV-02.md) — Handshake: player name, format fingerprint, ServerFull / FormatMismatch, TCP ↔ UDP link
- [SRV-03](SRV-03.md) — Typed messages: MessageDispatcher, send / broadcast / on<T>
- [SRV-07](SRV-07.md) — Tests: players, handshake, messages, codec, malformed input

## Read first

- [ADR 0037](../adr/0037-game-facing-networking.md)
- [reference 10](../engine/reference/10-network.md)

## To implement

- [ ] `PlayerTable` (private): PlayerId ↔ ConnectionId, names, allocation, removal
- [ ] `startServer(port)`, `stopServer()`, `connectTo(host, port, name)`, `disconnect()` on NetworkSession
- [ ] `players`, `localPlayer`, `name`, `kick`, `ping`, `stats(PlayerId)`
- [ ] Publish `PlayerJoined`, `PlayerLeft`, `JoinedServer`, `LeftServer`

## Done when

- Headless server + two clients: both PlayerJoined received, names correct, kick produces PlayerLeft{Kicked}

## Watch out

- `ConnectionId` never appears in a public signature
- Nothing opens automatically

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
