# ENG-19 · View<Ts...> and exclude<Xs...>

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **03 ECS** · status and owner live on the board.

## Goal

Queries over entities that have a set of components.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/ecs/View.hpp`

## Depends on

- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy

## Unlocks

- no task waits for this one

## Read first

- [ADR 0026](../adr/0026-registry-api.md)

## To implement

- [ ] `View<Ts...>` iterating the smallest storage
- [ ] `exclude<Xs...>`
- [ ] iterator yielding `tuple<Entity, Ts&...>`, `each`, `front`, `sizeHint`

## Done when

- Tests: structured bindings, exclude, empty view

## Watch out

- Writing component values during iteration is fine; structural changes are not

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
