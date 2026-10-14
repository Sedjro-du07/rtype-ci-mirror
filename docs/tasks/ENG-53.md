# ENG-53 · Unit tests: core and modules, with a test plugin and a test game

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **14 Tests & bench** · status and owner live on the board.

## Goal

Test setup and tests for core and modules, with a test plugin and a test game.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `tests/CMakeLists.txt`
- `tests/core/`
- `tests/module/`
- `tests/fixtures/`

## Depends on

- [ENG-14](ENG-14.md) — ModuleManager: load by kind and name, version check, reverse unload

## Unlocks

- no task waits for this one

## Read first

- [reference 01](../engine/reference/01-core.md)

## To implement

- [ ] `tests/CMakeLists.txt` with the chosen framework
- [ ] Fixtures: test plugin, test game
- [ ] Tests of ENG-04..15

## Done when

- `ctest` runs green on Linux and Windows

## Watch out

- Fixtures must build in both linking modes

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
