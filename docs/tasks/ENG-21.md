# ENG-21 · Component hooks onCreate / onReplace / onDestroy

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **03 ECS** · status and owner live on the board.

## Goal

onCreate / onReplace / onDestroy signals per component type.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/ecs/Registry.hpp` (shared with [ENG-18](ENG-18.md))

## Depends on

- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy
- [ENG-06](ENG-06.md) — Signal and RAII Subscription

## Unlocks

- [ENG-54](ENG-54.md) — Unit tests: ECS, reflection, commands, hooks

## Read first

- [ADR 0026](../adr/0026-registry-api.md)

## To implement

- [ ] `ComponentHooks` per ComponentId
- [ ] Fire from emplace, replace, remove, destroy and Commands::apply

## Done when

- Tests: each hook fires once per change, including via Commands and destroy

## Watch out

- onDestroy fires before removal so listeners can still read the component

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
