# ENG-38 · NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **10 Network** · status and owner live on the board.

## Goal

The core of NetworkSession: connections and the safe path from the network thread to the game thread.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/net/NetworkSession.hpp` (shared with [SRV-01](SRV-01.md))
- `src/net/NetworkSession.cpp` (shared with [SRV-01](SRV-01.md))

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
