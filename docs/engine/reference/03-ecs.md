# 3 · ECS

Status: **Locked**. ADRs 0012, 0013, 0014, 0015, 0016, 0026.
Folder: `engine/include/engine/ecs/`, `engine/src/ecs/`.

## Role

Store entities and their components, find them fast, change them safely. Holds data only; never
decides anything (systems decide).

---

## `struct Entity` — `ecs/Entity.hpp`

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

A handle never leaves the process (network identity is `NetworkId`, block 10).

## `class EntityAllocator` — `ecs/EntityAllocator.hpp` (internal to `Registry`)

```cpp
class EntityAllocator {
public:
    Entity create();                    // reuses a free slot (new generation) or appends one
    void   release(Entity e);           // marks the slot free, bumps its generation
    bool   alive(Entity e) const;       // index in range and generation matches
    size_t aliveCount() const;
};
```

## `class IStorage` — `ecs/IStorage.hpp`

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

## `class SparseSet<T>` — `ecs/SparseSet.hpp`

Paged sparse set (ADR 0013): `sparse` maps entity index → dense position in pages of 4096,
`dense` and `data` stay packed and aligned.

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

**Never keep a pointer or reference to a component across a structural change** (add, remove,
destroy): the dense array moves. Keep the `Entity` and look it up again.

## `class Registry` — `ecs/Registry.hpp`

One per scene. All template methods resolve `T` through the `TypeRegistry` (block 4); using a type
that was not registered is a programming error reported at the first use.

```cpp
class Registry {
public:
    explicit Registry(const TypeRegistry& types);

    // Entities
    Entity create();                                  // immediate; safe during iteration (touches no storage)
    void   destroy(Entity e);                         // immediate, recursive over Children. Engine/Commands only
    bool   alive(Entity e) const;
    size_t aliveCount() const;
    void   clear();                                   // destroys everything (scene unload)

    // Components, typed
    template <class T, class... Args> T& emplace(Entity e, Args&&... args); // immediate: use only outside iteration
    template <class T> T&       get(Entity e);        // asserts present
    template <class T> const T& get(Entity e) const;
    template <class T> T*       tryGet(Entity e);     // nullptr if absent
    template <class T> bool     has(Entity e) const;
    template <class T> T&       replace(Entity e, T value); // assigns and fires onReplace
    template <class T> void     remove(Entity e);     // immediate: use only outside iteration

    // Queries
    template <class... Ts> View<Ts...> view();                         // entities having all Ts
    template <class... Ts, class... Xs> View<Ts...> view(Exclude<Xs...>); // …and none of Xs

    // Type-erased access (scene loader, editor, scripts, replication)
    IStorage*       storage(ComponentId id);          // nullptr if no entity ever had it
    void*           getRaw(Entity e, ComponentId id);
    void*           emplaceRaw(Entity e, ComponentId id, const void* src);
    void            removeRaw(Entity e, ComponentId id);
    std::vector<ComponentId> componentsOf(Entity e) const;   // for the inspector and save()

    // Identity and hierarchy (ADR 0016)
    Entity find(std::string_view name) const;         // first entity whose Name matches, or null
    Entity find(const Uuid& uuid) const;              // indexed, O(1)
    std::vector<Entity> withTag(std::string_view tag) const;  // text tags
    void   setParent(Entity child, Entity parent, Vec2 localOffset = {}); // updates Parent and Children
    Entity parent(Entity e) const;
    std::span<const Entity> children(Entity e) const;

    // World singletons (ADR 0039): the one T of this world, on a hidden entity; game state of one room
    template <class T> T& singleton();
    template <class T> T* trySingleton();

    // Observer hooks (fired by emplace / replace / remove / destroy, including through Commands)
    template <class T> ComponentHooks& hooks();
    ComponentHooks& hooks(ComponentId id);
};

struct ComponentHooks {
    Signal<void(Registry&, Entity)> onCreate;   // after the component is added
    Signal<void(Registry&, Entity)> onReplace;  // after replace()
    Signal<void(Registry&, Entity)> onDestroy;  // before the component is removed
};
```

`Signal` and its RAII `Subscription` live in `core/Signal.hpp` (documented in 06-events).

## `class View<Ts...>` — `ecs/View.hpp`

```cpp
template <class... Xs> struct Exclude {};
template <class... Xs> inline constexpr Exclude<Xs...> exclude{};

template <class... Ts>
class View {
public:
    class iterator;                      // yields std::tuple<Entity, Ts&...>; works with structured bindings
    iterator begin(); iterator end();
    size_t sizeHint() const;             // size of the smallest storage
    template <class Fn> void each(Fn&& fn);   // fn(Entity, Ts&...)
    Entity front();                      // first match or Entity::null()
};
```

Iterates the smallest storage and checks the others in O(1). Reading and writing component values
inside the loop is safe; structural changes must go through `Commands`.

## `class Commands` — `ecs/Commands.hpp`

Command buffer (ADR 0015). Recorded during a stage, applied by the engine at the end of the stage.

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

---

## Benchmark — `engine/benchmarks/ecs_storage/`

Iterates 10 000 entities with one, two and three components, and adds/removes components, for
paged sparse sets, a plain sparse array and a minimal archetype store. Results go into the
comparative study (ADR 0013).
