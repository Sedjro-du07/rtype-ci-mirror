# ENG-17 · IStorage and paged SparseSet<T>

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **03 ECS** · **Wave 2** · status and owner live on the board.

## Goal

Component storage: type-erased interface and the paged sparse set behind it.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/ecs/IStorage.hpp`
- `include/engine/ecs/SparseSet.hpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/ecs/IStorage.hpp`

```cpp
class IStorage {
public:
    virtual ~IStorage() = default;
    virtual ComponentId id() const = 0;                  // id assigned by the TypeRegistry
    virtual bool   has(Entity) const = 0;
    virtual void   remove(Entity) = 0;                   // no-op if absent
    virtual size_t size() const = 0;
    virtual void   clear() = 0;
    virtual std::span<const Entity> entities() const = 0; // dense order
    virtual void*       getRaw(Entity) = 0;              // type-erased access (scene loader, editor, scripts)
    virtual const void* getRaw(Entity) const = 0;
    virtual void*  emplaceRaw(Entity, const void* src) = 0; // copy-constructs from src (TypeInfo::copy)
};
```

### `include/engine/ecs/SparseSet.hpp`

```cpp
template <class T>
class SparseSet final : public IStorage {
public:
    explicit SparseSet(ComponentId id);
    template <class... Args> T& emplace(Entity e, Args&&... args); // asserts absent; returns the new component
    T&       get(Entity e);                 // asserts present
    const T& get(Entity e) const;
    T*       tryGet(Entity e);              // nullptr if absent
    T&       replace(Entity e, T value);    // asserts present; assigns
    bool     has(Entity e) const override;
    void     remove(Entity e) override;     // swap with last, pop: O(1); invalidates references
    size_t   size() const override;
    std::span<const Entity> entities() const override;
    std::span<T>            components();   // dense array, same order as entities()
    // + IStorage raw overrides
};
```

## Depends on

- [ENG-16](ENG-16.md) — Entity and EntityAllocator (generations, free list)

## Unlocks

- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy
- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)
- [ENG-56](ENG-56.md) — ECS storage benchmark: sparse set vs sparse array vs archetypes

## Read first

- [ADR 0013](../adr/0013-sparse-set-storage.md)
- [ADR 0014](../adr/0014-type-erased-storage.md)

## To implement

- [ ] `IStorage` interface
- [ ] `SparseSet<T>` with pages of 4096, packed `dense` / `data`, swap-and-pop remove, raw overrides

## Done when

- Tests: add/remove/has across pages, iteration order, raw access
- Benchmark numbers recorded (ENG-56)

## Watch out

- Removing invalidates references: document it in the header

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
