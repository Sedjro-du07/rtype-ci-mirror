# ENG-18 · Registry: typed and type-erased access, identity, hierarchy

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **03 ECS** · status and owner live on the board.

## Goal

The per-scene world: entities, typed and raw component access, identity and hierarchy.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/ecs/Registry.hpp` (shared with [ENG-21](ENG-21.md))
- `src/ecs/Registry.cpp`

## Depends on

- [ENG-17](ENG-17.md) — IStorage and paged SparseSet<T>
- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)
- [ENG-06](ENG-06.md) — Signal and RAII Subscription

## Unlocks

- [ENG-19](ENG-19.md) — View<Ts...> and exclude<Xs...>
- [ENG-20](ENG-20.md) — Commands buffer (spawn, prefab spawn, destroy, add, set, remove)
- [ENG-21](ENG-21.md) — Component hooks onCreate / onReplace / onDestroy
- [ENG-39](ENG-39.md) — Replication: NetworkIdMap, net components, snapshot and input systems
- [ENG-41](ENG-41.md) — PrefabLibrary and instantiate
- [ENG-48](ENG-48.md) — IScripting, Script component, script systems
- [ENG-54](ENG-54.md) — Unit tests: ECS, reflection, commands, hooks

## Read first

- [ADR 0026](../adr/0026-registry-api.md)
- [reference 03](../engine/reference/03-ecs.md)

## To implement

- [ ] Entities: create, destroy (recursive over Children), alive, clear
- [ ] Typed: emplace, get, tryGet, has, replace, remove via TypeRegistry ids
- [ ] Raw: storage, getRaw, emplaceRaw, removeRaw, componentsOf
- [ ] Identity: find(name), find(Uuid) indexed, withTag
- [ ] Hierarchy: setParent, parent, children

## Done when

- Tests: every method, unregistered type error message, recursive destroy

## Watch out

- Hooks fire on every path, including raw and Commands

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
