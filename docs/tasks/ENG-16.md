# ENG-16 · Entity and EntityAllocator (generations, free list)

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **03 ECS** · status and owner live on the board.

## Goal

Entity handles with generations so stale handles are detected.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/ecs/Entity.hpp`
- `include/engine/ecs/EntityAllocator.hpp`
- `src/ecs/EntityAllocator.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/ecs/Entity.hpp`

```cpp
struct Entity {
    uint32_t index = UINT32_MAX;       // slot in the storages
    uint32_t generation = 0;           // incremented each time the slot is reused
    static constexpr Entity null() noexcept;                // the invalid handle
    constexpr bool isNull() const noexcept;
    friend constexpr bool operator==(Entity, Entity) = default;
};
template <> struct std::hash<engine::Entity>;              // for unordered containers
```

### `include/engine/ecs/EntityAllocator.hpp`

```cpp
class EntityAllocator {
public:
    Entity create();                    // reuses a free slot (new generation) or appends one
    void   release(Entity e);           // marks the slot free, bumps its generation
    bool   alive(Entity e) const;       // index in range and generation matches
    size_t aliveCount() const;
};
```

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
