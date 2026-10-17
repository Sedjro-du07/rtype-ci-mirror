# `engine/src/systems/`

Headers in this folder are **private** to the engine: they are not under `include/`, so games cannot include them.

Placeholder created from the architecture. Replace this file's list as files are written.

| File to create | Task | Details |
|---|---|---|
| `AnimationSystem.cpp` | ENG-52 — Render systems: sprites (interpolated), text, animation, debug colliders | `docs/tasks/ENG-52.md` |
| `CollisionSystem.cpp` | ENG-51 — Collision system: uniform grid, AABB, layers, Collision events | `docs/tasks/ENG-51.md` |
| `DebugOverlaySystem.cpp` | ENG-47 — Metrics and engine.debug_overlay (ImGui, ImPlot) | `docs/tasks/ENG-47.md` |
| `MovementSystems.cpp` | ENG-50 — Systems: previous_position, movement, hierarchy, parallax, lifetime | `docs/tasks/ENG-50.md` |
| `NetSystems.cpp` | ENG-39 — Replication: NetworkIdMap, net components, snapshot and input systems | `docs/tasks/ENG-39.md` |
| `RegisterEngineSystems.cpp` | ENG-50 — Systems: previous_position, movement, hierarchy, parallax, lifetime | `docs/tasks/ENG-50.md` |
| `RegisterEngineTypes.cpp` | ENG-25 — Engine components and their registration | `docs/tasks/ENG-25.md` |
| `RegisterEngineTypes.hpp` | ENG-25 — Engine components and their registration | `docs/tasks/ENG-25.md` |
| `RenderSystems.cpp` | ENG-52 — Render systems: sprites (interpolated), text, animation, debug colliders | `docs/tasks/ENG-52.md` |

Design: `docs/engine/reference/`. Rules: `docs/engine/folder-tree.md`.
