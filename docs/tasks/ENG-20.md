# ENG-20 · Commands buffer (spawn, prefab spawn, destroy, add, set, remove)

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **03 ECS** · status and owner live on the board.

## Goal

Deferred structural changes so systems can spawn and destroy while iterating (ADR 0015).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/ecs/Commands.hpp`
- `src/ecs/Commands.cpp`

## Depends on

- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy

## Unlocks

- [ENG-50](ENG-50.md) — Systems: previous_position, movement, hierarchy, parallax, lifetime
- [ENG-54](ENG-54.md) — Unit tests: ECS, reflection, commands, hooks

## Read first

- [ADR 0015](../adr/0015-command-buffer.md)

## To implement

- [ ] spawn (empty, prefab), destroy, add, set, remove, setParent, addRaw
- [ ] `apply` in recording order, skipping dead entities

## Done when

- Tests: spawn inside a view loop, destroy then add on the same entity, prefab spawn

## Watch out

- The Entity returned by spawn is valid immediately but has components only after apply

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
