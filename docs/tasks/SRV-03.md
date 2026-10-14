# SRV-03 · Typed messages: MessageDispatcher, send / broadcast / on<T>

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **02 Messages** · status and owner live on the board.

## Goal

Typed messages for games: send, broadcast and on<T> callbacks.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/net/MessageDispatcher.hpp`
- `src/net/MessageDispatcher.cpp`

## Depends on

- [SRV-01](SRV-01.md) — Players: PlayerTable, startServer / connectTo, player events, names, kick, ping
- [SRV-04](SRV-04.md) — Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits
- [SRV-05](SRV-05.md) — ReliableChannel: acknowledged and resent UDP messages
- [SRV-06](SRV-06.md) — StreamFramer: TCP message framing with size limit

## Unlocks

- [SRV-07](SRV-07.md) — Tests: players, handshake, messages, codec, malformed input

## Read first

- [ADR 0037](../adr/0037-game-facing-networking.md)
- [reference 10](../engine/reference/10-network.md)

## To implement

- [ ] `MessageDispatcher` (private): message id ↔ TypeInfo, callback lists per id
- [ ] `send<T>` ×2, `broadcast<T>` choosing the transport declared on the message
- [ ] `on<T>` ×2 returning a Subscription
- [ ] Call callbacks during `poll()` on the loop thread; catch exceptions and log with the message name

## Done when

- Tests: Chat over stream, WaveCleared over reliable, Ping over unreliable; exception in a callback does not stop the server

## Watch out

- Message ids follow `TypeRegistry::messages()` order
- Callbacks must check `Entity::isNull()`

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
