# SRV-07 · Tests: players, handshake, messages, codec, malformed input

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **04 Tests** · **Wave 8** · status and owner live on the board.

## Goal

Prove the server layer is correct and robust.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `tests/net/players/`
- `tests/net/messages/`
- `tests/reflection/codec/`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `tests/`

```text
tests/net/players/        JoinLeaveTest, KickTest, TimeoutTest, NamesTest
tests/net/players/        HandshakeTest (FormatMismatch, ServerFull, bad name, UDP link)
tests/net/messages/       UnreliableTest, ReliableUnderLossTest (20 % loss), StreamTest, CallbackExceptionTest
tests/reflection/codec/   RoundTripTest (every FieldKind), LimitsTest, EntityMappingTest, FuzzTest (10 000 inputs)
```

## Depends on

- [SRV-01](SRV-01.md) — Players: PlayerTable, startServer / connectTo, player events, names, kick, ping
- [SRV-02](SRV-02.md) — Handshake: player name, format fingerprint, ServerFull / FormatMismatch, TCP ↔ UDP link
- [SRV-03](SRV-03.md) — Typed messages: MessageDispatcher, send / broadcast / on<T>
- [SRV-04](SRV-04.md) — Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits
- [SRV-05](SRV-05.md) — ReliableChannel: acknowledged and resent UDP messages
- [SRV-06](SRV-06.md) — StreamFramer: TCP message framing with size limit

## Unlocks

- no task waits for this one

## Read first

- [ADR 0037](../adr/0037-game-facing-networking.md)
- [guide 12](../engine/guide/12-networking.md)

## To implement

- [ ] Players: join, leave, kick, timeout
- [ ] Handshake: mismatch, full, bad name
- [ ] Messages on the three transports
- [ ] Codec fuzzing and malformed input

## Done when

- All green in CI; fuzz test runs at least 10 000 random inputs

## Watch out

- Loopback only, port 0

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
