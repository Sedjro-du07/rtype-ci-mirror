# ENG-16 · Entity and EntityAllocator (generations, free list)

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **03 ECS** · status and owner live on the board.

## Goal

Entity handles with generations so stale handles are detected.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/ecs/Entity.hpp`
- `include/engine/ecs/EntityAllocator.hpp`
- `src/ecs/EntityAllocator.cpp`

## Depends on

- nothing: can start now

## Unlocks

- [ENG-17](ENG-17.md) — IStorage and paged SparseSet<T>

## Read first

- [ADR 0012](../adr/0012-entity-handle.md)
- [reference 03](../engine/reference/03-ecs.md)

## To implement

- [ ] `Entity` (index, generation, null, isNull, ==, std::hash)
- [ ] `EntityAllocator`: free list, generation bump on release

## Done when

- Tests: reused slot gets a new generation; stale handle is not alive

## Watch out

- An Entity never leaves the process (network uses NetworkId)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
