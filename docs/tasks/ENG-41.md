# ENG-41 · PrefabLibrary and instantiate

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **11 Scenes & assets** · status and owner live on the board.

## Goal

Prefabs: parsed once, instantiated with overrides and children.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/scene/Prefab.hpp`
- `src/scene/PrefabLibrary.cpp`

## Depends on

- [ENG-23](ENG-23.md) — YAML wrappers and field codecs
- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy
- [ENG-40](ENG-40.md) — AssetManager: handles, cache, refcount, fallback, hot reload

## Unlocks

- [ENG-42](ENG-42.md) — SceneLoader: two passes, errors with file:line, save

## Read first

- [ADR 0034](../adr/0034-scenes-prefabs-assets.md)

## To implement

- [ ] `PrefabLibrary::get` (cached), `findByKey`, `instantiate`, `clear`

## Done when

- Tests: overrides, nested children with offsets

## Watch out

- `Prefab::key` is used by replication: same hash on every machine

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
