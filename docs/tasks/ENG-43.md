# ENG-43 · Scene and SceneManager: switch, persistent entities

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **11 Scenes & assets** · **Wave 9** · status and owner live on the board.

## Goal

One active scene, deferred switching, persistent entities.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/scene/Scene.hpp`
- `include/engine/scene/SceneManager.hpp`
- `src/scene/SceneManager.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/scene/Scene.hpp`

```cpp
class Scene {
public:
    const std::string& name() const;
    const std::filesystem::path& path() const;
    Registry& registry();
};
```

### `include/engine/scene/SceneManager.hpp`

```cpp
class SceneManager {
public:
    SceneManager(const TypeRegistry&, AssetManager&, EventBus&, Logger&);
    Result<void> load(const std::filesystem::path& file);   // engine only (first scene): immediate
    void switchTo(const std::filesystem::path& file);      // deferred to the end of the frame
    void markPersistent(Entity e);                         // copied into the next scene on switch (the player)
    Result<void> save(const std::filesystem::path& file) const;
    Registry& world();                                     // active scene's registry
    std::string_view activeName() const;
    PrefabLibrary& prefabs();
    void applyPending(Context& ctx);                       // engine only: SceneUnloading → clear → load → SceneLoaded
    void clear();                                          // engine only: shutdown, before the game module unloads
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
