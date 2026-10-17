# ENG-18 · Registry: typed and type-erased access, identity, hierarchy

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **03 ECS** · **Wave 4** · status and owner live on the board.

## Goal

The per-scene world: entities, typed and raw component access, identity and hierarchy.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/ecs/Registry.hpp` (shared with [ENG-21](ENG-21.md))
- `include/engine/ecs/Registry.inl`
- `src/ecs/Registry.cpp`
- `include/engine/components/Identity.hpp`
- `include/engine/components/Hierarchy.hpp`
- `src/components/Identity.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/ecs/Registry.hpp`

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

### `include/engine/components/Identity.hpp`

```cpp
struct Name             { std::string value; };                           ENGINE_COMPONENT(Name)
struct Uuid             { uint64_t hi = 0, lo = 0; static Uuid generate(); bool operator==(const Uuid&) const = default; };
                                                                          ENGINE_COMPONENT(Uuid)
struct Tags             { std::vector<uint64_t> ids; bool has(std::string_view tag) const; };
                                                                          ENGINE_COMPONENT(Tags)
template <> struct std::hash<engine::Uuid>;      // for the uuid index
```

### `include/engine/components/Hierarchy.hpp`

```cpp
struct Parent           { Entity value; Vec2 offset; };                   ENGINE_COMPONENT(Parent)
struct Children         { std::vector<Entity> list; };                    ENGINE_COMPONENT(Children)
```

## Depends on

- [ENG-17](ENG-17.md) — IStorage and paged SparseSet<T>
- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)
- [ENG-06](ENG-06.md) — Signal and RAII Subscription
- [ENG-60](ENG-60.md) — OS layer: secure random, user data folder, executable folder, quit signals

## Unlocks

- [ENG-19](ENG-19.md) — View<Ts...> and exclude<Xs...>
- [ENG-20](ENG-20.md) — Commands buffer (spawn, prefab spawn, destroy, add, set, remove)
- [ENG-21](ENG-21.md) — Component hooks onCreate / onReplace / onDestroy
- [ENG-39](ENG-39.md) — Replication: NetworkIdMap, net components, snapshot and input systems
- [ENG-41](ENG-41.md) — PrefabLibrary and instantiate
- [ENG-48](ENG-48.md) — IScripting, Script component, script systems
- [ENG-54](ENG-54.md) — Unit tests: ECS, reflection, commands, hooks
- [SRV-11](SRV-11.md) — RoomRuntime and RoomMailbox: one isolated game, lobby ↔ room messages

## Read first

- [ADR 0026](../adr/0026-registry-api.md)
- [ADR 0039](../adr/0039-room-api.md)
- [reference 03](../engine/reference/03-ecs.md)

## To implement

- [ ] Entities: create, destroy (recursive over Children), alive, clear
- [ ] Typed: emplace, get, tryGet, has, replace, remove via TypeRegistry ids
- [ ] Raw: storage, getRaw, emplaceRaw, removeRaw, componentsOf
- [ ] Identity: find(name), find(Uuid) indexed, withTag
- [ ] Hierarchy: setParent, parent, children
- [ ] World singletons: `singleton<T>()`, `trySingleton<T>()` on a hidden entity (ADR 0039)
- [ ] Identity and hierarchy components `Name`, `Uuid` (`generate()` with `os::secureRandom`), `Tags`, `Parent`, `Children` (moved here from ENG-25: they need no rendering)

## Done when

- Tests: every method, unregistered type error message, recursive destroy

## Watch out

- Hooks fire on every path, including raw and Commands

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
