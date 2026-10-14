# ENG-40 · AssetManager: handles, cache, refcount, fallback, hot reload

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **11 Scenes & assets** · status and owner live on the board.

## Goal

Load each resource once behind a handle, with fallbacks.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/assets/AssetManager.hpp`
- `src/assets/AssetManager.cpp`

## Depends on

- [ENG-30](ENG-30.md) — Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform
- [ENG-34](ENG-34.md) — IAudio, PlayParams, buses, captions
- [ENG-48](ENG-48.md) — IScripting, Script component, script systems

## Unlocks

- [ENG-41](ENG-41.md) — PrefabLibrary and instantiate

## Read first

- [ADR 0034](../adr/0034-scenes-prefabs-assets.md)
- [reference 11](../engine/reference/11-scenes-assets.md)

## To implement

- [ ] texture, font, sound, music, script with cache
- [ ] pathOf, retain / release, collectUnused, reloadChanged, resolve (reject `../`), stats

## Done when

- Missing texture shows the checkerboard and logs once

## Watch out

- On the server, handles are valid but nothing is decoded

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
