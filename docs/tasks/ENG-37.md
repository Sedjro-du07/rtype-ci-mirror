# ENG-37 · network_asio plugin: UDP and TCP on an I/O thread

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **10 Network** · status and owner live on the board.

## Goal

The transport plugin: UDP and TCP on an I/O thread with standalone Asio (ADR 0037: TCP from Part 1).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/network_asio/CMakeLists.txt`
- `plugins/network_asio/AsioNetwork.hpp`
- `plugins/network_asio/AsioNetwork.cpp`

## Depends on

- [ENG-36](ENG-36.md) — INetwork and network types
- [ENG-02](ENG-02.md) — CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher

## Unlocks

- [SRV-06](SRV-06.md) — StreamFramer: TCP message framing with size limit

## Read first

- [ADR 0033](../adr/0033-engine-networking.md)
- [ADR 0037](../adr/0037-game-facing-networking.md)

## To implement

- [ ] One `io_context` on its own thread
- [ ] UDP socket: open, send (thread-safe copy), receiver callback
- [ ] TCP acceptor and streams: listen, connect, send, receiver, close

## Done when

- Loopback test: datagrams and stream bytes arrive; closing during traffic does not crash

## Watch out

- The receiver runs on the I/O thread: only push into the session inbox

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
