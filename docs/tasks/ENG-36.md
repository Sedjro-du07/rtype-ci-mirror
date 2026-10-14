# ENG-36 · INetwork and network types

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **10 Network** · status and owner live on the board.

## Goal

Network vocabulary and the byte transport interface.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/net/NetTypes.hpp`
- `include/engine/net/INetwork.hpp`

## Depends on

- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE

## Unlocks

- [ENG-15](ENG-15.md) — Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting
- [ENG-37](ENG-37.md) — network_asio plugin: UDP and TCP on an I/O thread
- [ENG-38](ENG-38.md) — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts

## Read first

- [ADR 0033](../adr/0033-engine-networking.md)
- [reference 10](../engine/reference/10-network.md)

## To implement

- [ ] `PlayerId`, `ConnectionId`, `Endpoint` (`resolve`, `toString`), `Datagram`, `DisconnectReason`, `NetConfig`, `ConnectionStats`
- [ ] `INetwork` (UDP and TCP)

## Done when

- Endpoint resolves IPv4 and IPv6 hosts

## Watch out

- `ConnectionId` is engine-internal; games use `PlayerId`

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
