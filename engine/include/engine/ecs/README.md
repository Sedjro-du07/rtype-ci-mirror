# `engine/include/engine/ecs/`

Entities, storages, Registry, View, Commands.

Placeholder created from the engine architecture (2026-10-09). Replace this file's list as files are written.

| File to create | Task |
|---|---|
| `Commands.hpp` | ENG-20 — Commands buffer (spawn, prefab spawn, destroy, add, set, remove) |
| `Entity.hpp` | ENG-16 — Entity and EntityAllocator (generations, free list) |
| `EntityAllocator.hpp` | ENG-16 — Entity and EntityAllocator (generations, free list) |
| `IStorage.hpp` | ENG-17 — IStorage and paged SparseSet<T> |
| `Registry.hpp` | ENG-18 — Registry: typed and type-erased access, identity, hierarchy, ENG-21 — Component hooks onCreate / onReplace / onDestroy |
| `SparseSet.hpp` | ENG-17 — IStorage and paged SparseSet<T> |
| `View.hpp` | ENG-19 — View<Ts...> and exclude<Xs...> |

Design: `docs/engine/reference/`. Rules: `docs/engine/folder-tree.md`.
