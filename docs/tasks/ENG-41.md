# ENG-41 · PrefabLibrary and instantiate

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **11 Scenes & assets** · **Wave 7** · status and owner live on the board.

## Goal

Prefabs: parsed once, instantiated with overrides and children.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/scene/Prefab.hpp`
- `src/scene/PrefabLibrary.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/scene/Prefab.hpp`

```cpp
struct Prefab {
    std::string                     path;
    uint32_t                        key;       // fnv1a(path), used by NetPrefab
    std::vector<ComponentData>      components;// parsed values, already type-checked
    std::vector<PrefabChild>        children;  // nested prefabs or inline entities, with local offsets
};

class PrefabLibrary {
public:
    PrefabLibrary(const TypeRegistry& types, AssetManager& assets);
    Result<const Prefab*> get(std::string_view path);   // parses on first use, cached
    const Prefab* findByKey(uint32_t key) const;        // replication spawn
    Entity instantiate(Registry& r, const Prefab& p, Vec2 position,
                       std::span<const ComponentData> overrides = {}); // creates the entity and its children
    void clear();
};

struct ComponentData {                 // one parsed component value, ready to copy into a registry
    ComponentId            id;
    std::vector<std::byte> value;      // constructed with TypeInfo::construct, filled by fromYaml
    int                    line = 0;   // for error messages
};
struct PrefabChild {
    std::string                prefab;      // nested prefab path, or empty for an inline entity
    std::vector<ComponentData> components;  // inline components or overrides of the nested prefab
    Vec2                       offset;      // local offset from the parent
};
```

## Depends on

- [ENG-23](ENG-23.md) — YAML wrappers and field codecs
- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy
- [ENG-40](ENG-40.md) — AssetManager: handles, cache, refcount, fallback, hot reload

## Unlocks

- [ENG-42](ENG-42.md) — SceneLoader: two passes, errors with file:line, save

## Read first

- [ADR 0034](../adr/0034-scenes-prefabs-assets.md)

## To implement

- [ ] `PrefabLibrary::get` (cached), `findByKey`, `instantiate`, `clear`

## Done when

- Tests: overrides, nested children with offsets

## Watch out

- `Prefab::key` is used by replication: same hash on every machine

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
