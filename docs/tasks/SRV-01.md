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
