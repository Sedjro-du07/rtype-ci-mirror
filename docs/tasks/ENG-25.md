# ENG-25 · Engine components and their registration

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **04 Reflection** · status and owner live on the board.

## Goal

The engine's built-in components and their registration.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/components/Position.hpp`
- `include/engine/components/Velocity.hpp`
- `include/engine/components/Sprite.hpp`
- `include/engine/components/Animation.hpp`
- `include/engine/components/Text.hpp`
- `include/engine/components/Collider.hpp`
- `include/engine/components/Lifetime.hpp`
- `include/engine/components/Parallax.hpp`
- `include/engine/components/Identity.hpp`
- `include/engine/components/Hierarchy.hpp`
- `include/engine/components/Script.hpp`
- `src/systems/RegisterEngineTypes.cpp`

## Depends on

- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)
- [ENG-30](ENG-30.md) — Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform

## Unlocks

- [ENG-50](ENG-50.md) — Systems: previous_position, movement, hierarchy, parallax, lifetime
- [ENG-51](ENG-51.md) — Collision system: uniform grid, AABB, layers, Collision events
- [ENG-52](ENG-52.md) — Render systems: sprites (interpolated), text, animation, debug colliders

## Read first

- [ADR 0016](../adr/0016-identity-as-components.md)
- [reference 13](../engine/reference/13-components-systems.md)

## To implement

- [ ] Headers of reference 13 with `ENGINE_COMPONENT`
- [ ] `registerEngineTypes`: fields and flags (Position/Velocity/Sprite Replicated, PreviousPosition Transient…) and engine events

## Done when

- Inspector-style dump (`types` console command) lists every engine component with its fields

## Watch out

- Names are reserved: a game type with the same name is a boot error

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
