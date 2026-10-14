# SRV-15 · Tests: parallel rooms, isolation, crash in a room, party and room flows

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **04 Tests** · **Wave 16** · status and owner live on the board.

## Goal

Prove rooms are isolated, parallel and robust.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `tests/room/`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `tests/room/`

```text
tests/room/  ParallelRoomsTest (no shared entities, events, ids), RoomCrashTest (RoomFailed, others continue),
             PartyFlowTest (invite, answer, leave, leader change, leader disconnect),
             RoomFlowTest (create, move, finish, close, maxRooms), SystemCopyPerRoomTest — run with ThreadSanitizer
```

## Depends on

- [SRV-13](SRV-13.md) — Lobby and RoomHandle: the server game API, lobby loop in Engine
- [SRV-14](SRV-14.md) — Routing per room: PlayerRouting, RoomNetwork, ClientLobby, replication reset

## Unlocks

- no task waits for this one

## Read first

- [ADR 0038](../adr/0038-rooms-shared-host.md)
- [ADR 0039](../adr/0039-room-api.md)
- [reference 14](../engine/reference/14-rooms.md)

## To implement

- [ ] Parallel rooms (no shared entities, events, ids)
- [ ] Crash in one room → RoomFailed, others continue
- [ ] Party flows (invite, answer, leave, leader change, disconnect of a leader)
- [ ] Room flows (create, move, finish, close, maxRooms)
- [ ] Systems with state get one copy per room

## Done when

- All green in CI with thread sanitizer enabled

## Watch out

- Run the room tests under ThreadSanitizer: data races are the main risk

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
