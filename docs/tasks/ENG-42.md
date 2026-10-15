# ENG-42 · SceneLoader: two passes, errors with file:line, save

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **11 Scenes & assets** · **Wave 8** · status and owner live on the board.

## Goal

Scene files in two passes with precise errors, and saving back.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/scene/SceneLoader.hpp`
- `src/scene/SceneLoader.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/scene/SceneLoader.hpp`

```cpp
class SceneLoader {
public:
    SceneLoader(const TypeRegistry& types, AssetManager& assets, PrefabLibrary& prefabs);
    Result<std::unique_ptr<Scene>> load(const std::filesystem::path& file);
    // Pass 1: parse; for each entity create it, resolve its prefab, apply overrides, record uuid → entity.
    // Pass 2: link Parent / Children and Entity fields by uuid (declaration order does not matter).
    // Then request the assets. Any error returns file:line and leaves no partial scene behind.
    Result<void> save(const Scene& scene, const std::filesystem::path& file) const;
    // Writes uuids, prefab references and only the components that differ from the prefab; skips Transient types.
};
```

## Depends on

- [ENG-41](ENG-41.md) — PrefabLibrary and instantiate

## Unlocks

- [ENG-43](ENG-43.md) — Scene and SceneManager: switch, persistent entities

## Read first

- [ADR 0034](../adr/0034-scenes-prefabs-assets.md)
- [guide 10](../engine/guide/10-scenes-prefabs-assets.md)

## To implement

- [ ] Pass 1 entities + prefabs + overrides; pass 2 links by uuid
- [ ] Errors with file:line, no partial scene
- [ ] `save`: only differences from the prefab, skip Transient

## Done when

- Tests: forward references, unknown component suggestion, round trip

## Watch out

- A failed load leaves the current scene untouched

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
