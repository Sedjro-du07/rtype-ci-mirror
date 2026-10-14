# ENG-50 · Systems: previous_position, movement, hierarchy, parallax, lifetime

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **13 Engine systems** · **Wave 6** · status and owner live on the board.

## Goal

Movement-related built-in systems and the registration of all engine systems.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/systems/MovementSystems.cpp`
- `src/systems/RegisterEngineSystems.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/systems/MovementSystems.cpp`

```cpp
void previousPosition(Context& ctx);   // "engine.previous_position" FixedUpdate, all
void movement(Context& ctx);           // "engine.movement"          FixedUpdate, all
void hierarchy(Context& ctx);          // "engine.hierarchy"         FixedUpdate, all
void parallax(Context& ctx);           // "engine.parallax"          FixedUpdate, client
void lifetime(Context& ctx);           // "engine.lifetime"          FixedUpdate, all
```

### `src/systems/RegisterEngineSystems.cpp`

```cpp
void registerEngineSystems(Scheduler& s, Role role);   // every engine system with its name, stage, roles and order (reference 13)
```

## Depends on

- [ENG-25](ENG-25.md) — Engine components and their registration
- [ENG-26](ENG-26.md) — Scheduler: stages, names, after/before, roles, stats, failure policy
- [ENG-20](ENG-20.md) — Commands buffer (spawn, prefab spawn, destroy, add, set, remove)

## Unlocks

- no task waits for this one

## Read first

- [reference 13](../engine/reference/13-components-systems.md)

## To implement

- [ ] previous_position, movement, hierarchy, parallax, lifetime
- [ ] `registerEngineSystems` with names and order of reference 13

## Done when

- Tests: movement uses fixedDelta; children follow parents; lifetime destroys through Commands

## Watch out

- Names are fixed: games order against them

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
