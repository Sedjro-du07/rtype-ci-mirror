# ENG-45 · CVars: typed variables, flags, ranges, archive

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **12 Services** · status and owner live on the board.

## Goal

Typed console variables with flags, ranges and persistence.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/services/CVars.hpp`
- `src/services/CVars.cpp`

## Depends on

- [ENG-06](ENG-06.md) — Signal and RAII Subscription
- [ENG-23](ENG-23.md) — YAML wrappers and field codecs

## Unlocks

- [ENG-46](ENG-46.md) — Console and built-in commands

## Read first

- [ADR 0035](../adr/0035-services-and-tools.md)

## To implement

- [ ] `CVar<T>` with onChange
- [ ] `CVars::define`, set, get, find, forEach
- [ ] `loadArchive` / `saveArchive` (Archive flag)
- [ ] Engine CVars list of reference 12

## Done when

- Tests: range check, archive round trip

## Watch out

- `sim_tick_rate` is read-only

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
