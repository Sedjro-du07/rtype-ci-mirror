# ENG-44 · Logger and sinks (console, file, developer console)

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **12 Services** · status and owner live on the board.

## Goal

Thread-safe logging with tags and several sinks.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/services/Logger.hpp`
- `src/services/Logger.cpp`

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError

## Unlocks

- [ENG-14](ENG-14.md) — ModuleManager: load by kind and name, version check, reverse unload
- [ENG-26](ENG-26.md) — Scheduler: stages, names, after/before, roles, stats, failure policy

## Read first

- [ADR 0035](../adr/0035-services-and-tools.md)
- [reference 12](../engine/reference/12-services.md)

## To implement

- [ ] Levels, `std::format` API, `tagged`
- [ ] ConsoleSink (colours), FileSink (rotating), ConsoleUiSink

## Done when

- Logs from the network thread and the game thread never interleave mid-line

## Watch out

- Never a singleton

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
