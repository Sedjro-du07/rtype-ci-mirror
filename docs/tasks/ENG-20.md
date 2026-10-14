# ENG-20 · Commands buffer (spawn, prefab spawn, destroy, add, set, remove)

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **03 ECS** · **Wave 5** · status and owner live on the board.

## Goal

Deferred structural changes so systems can spawn and destroy while iterating (ADR 0015).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/ecs/Commands.hpp`
- `src/ecs/Commands.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/ecs/Commands.hpp`

```cpp
class Commands {
public:
    Entity spawn();                                         // entity created now, components added at apply()
    Entity spawn(std::string_view prefab, Vec2 position);   // instantiate a prefab at apply() (block 11)
    void   destroy(Entity e);                               // destroyed (with children) at apply()
    template <class T> void add(Entity e, T value);
    template <class T> void set(Entity e, T value);         // add or replace
    template <class T> void remove(Entity e);
    void   setParent(Entity child, Entity parent, Vec2 localOffset = {}); // Entity::null() detaches
    void   addRaw(Entity e, ComponentId id, std::vector<std::byte> value); // scripts, replication
    bool   empty() const;
    void   apply(Registry& registry, SceneManager& scenes); // engine only; in recording order; skips dead entities
};
```

## Depends on

- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy

## Unlocks

- [ENG-50](ENG-50.md) — Systems: previous_position, movement, hierarchy, parallax, lifetime
- [ENG-54](ENG-54.md) — Unit tests: ECS, reflection, commands, hooks

## Read first

- [ADR 0015](../adr/0015-command-buffer.md)

## To implement

- [ ] spawn (empty, prefab), destroy, add, set, remove, setParent, addRaw
- [ ] `apply` in recording order, skipping dead entities

## Done when

- Tests: spawn inside a view loop, destroy then add on the same entity, prefab spawn

## Watch out

- The Entity returned by spawn is valid immediately but has components only after apply

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
