# `engine/include/engine/ecs/`

Entities, storages, Registry, View, Commands.

Placeholder created from the architecture. Replace this file's list as files are written.

| File to create | Task | Details |
|---|---|---|
| `Commands.hpp` | ENG-20 — Commands buffer (spawn, prefab spawn, destroy, add, set, remove) | `docs/tasks/ENG-20.md` |
| `Entity.hpp` | ENG-16 — Entity and EntityAllocator (generations, free list) | `docs/tasks/ENG-16.md` |
| `EntityAllocator.hpp` | ENG-16 — Entity and EntityAllocator (generations, free list) | `docs/tasks/ENG-16.md` |
| `IStorage.hpp` | ENG-17 — IStorage and paged SparseSet<T> | `docs/tasks/ENG-17.md` |
| `Registry.hpp` | ENG-18, ENG-21 — Registry: typed and type-erased access, identity, hierarchy | `docs/tasks/ENG-18.md` |
| `SparseSet.hpp` | ENG-17 — IStorage and paged SparseSet<T> | `docs/tasks/ENG-17.md` |
| `View.hpp` | ENG-19 — View<Ts...> and exclude<Xs...> | `docs/tasks/ENG-19.md` |

Design: `docs/engine/reference/`. Rules: `docs/engine/folder-tree.md`.
