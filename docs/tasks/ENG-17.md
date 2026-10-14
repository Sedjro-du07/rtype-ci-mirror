# ENG-17 · IStorage and paged SparseSet<T>

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **03 ECS** · status and owner live on the board.

## Goal

Component storage: type-erased interface and the paged sparse set behind it.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/ecs/IStorage.hpp`
- `include/engine/ecs/SparseSet.hpp`

## Depends on

- [ENG-16](ENG-16.md) — Entity and EntityAllocator (generations, free list)

## Unlocks

- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy
- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)
- [ENG-56](ENG-56.md) — ECS storage benchmark: sparse set vs sparse array vs archetypes

## Read first

- [ADR 0013](../adr/0013-sparse-set-storage.md)
- [ADR 0014](../adr/0014-type-erased-storage.md)

## To implement

- [ ] `IStorage` interface
- [ ] `SparseSet<T>` with pages of 4096, packed `dense` / `data`, swap-and-pop remove, raw overrides

## Done when

- Tests: add/remove/has across pages, iteration order, raw access
- Benchmark numbers recorded (ENG-56)

## Watch out

- Removing invalidates references: document it in the header

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
