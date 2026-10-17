# ENG-43 · Scene and SceneManager: switch, persistent entities

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **11 Scenes & assets** · **Wave 9** · status and owner live on the board.

## Goal

One active scene, deferred switching, persistent entities.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/scene/Scene.hpp`
- `include/engine/scene/SceneManager.hpp`
- `src/scene/Scene.cpp`
- `src/scene/SceneManager.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/scene/Scene.hpp`

```cpp
class Scene {                                      // not copyable: owned through std::unique_ptr
public:
    Scene(const TypeRegistry& types, std::string name, std::filesystem::path path);
    const std::string& name() const;
    const std::filesystem::path& path() const;
    Registry& registry(); const Registry& registry() const;
    void setUuid(Entity, const Uuid&);  std::optional<Uuid> uuidOf(Entity) const;     // filled by the loader
    void setPrefab(Entity, std::string); std::string_view prefabOf(Entity) const;     // "" if none
    void markGenerated(Entity);         bool generated(Entity) const;   // created as a prefab's child
};
```

### `include/engine/scene/SceneManager.hpp`

```cpp
class SceneManager {
public:
    SceneManager(const TypeRegistry&, AssetManager&, EventBus&, Logger&);
    Result<void> load(const std::filesystem::path& file);   // engine only (first scene): immediate
    void switchTo(const std::filesystem::path& file);      // deferred to applyPending (end of the frame)
    void markPersistent(Entity e);                         // copied with its children into the next scene
    Result<void> save(const std::filesystem::path& file) const;
    Registry& world();                                     // active scene's registry
    const Scene& active() const;
    std::string_view activeName() const;
    PrefabLibrary& prefabs();
    bool hasPending() const;
    void applyPending();     // engine only: load the next scene; on success emit SceneUnloading (old world
                             // still readable), copy the persistent entities, swap, emit SceneLoaded
    void clear();            // engine only: shutdown, before the game module unloads
};
```

## Depends on

- [ENG-42](ENG-42.md) — SceneLoader: two passes, errors with file:line, save
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-55](ENG-55.md) — Unit tests: scheduler, events, input, scenes, session
- [ENG-59](ENG-59.md) — Editor (Track 1): hierarchy, inspector, scene save
- [SRV-11](SRV-11.md) — RoomRuntime and RoomMailbox: one isolated game, lobby ↔ room messages

## Read first

- [ADR 0034](../adr/0034-scenes-prefabs-assets.md)

## To implement

- [ ] `load`, `switchTo` (end of frame), `markPersistent`, `save`, `world`, `applyPending`, `clear`

## Done when

- Switching keeps marked entities; SceneLoaded / SceneUnloading are published

## Watch out

- Systems must re-read `world()` after a switch

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
