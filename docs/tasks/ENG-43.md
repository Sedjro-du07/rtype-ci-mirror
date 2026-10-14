# ENG-43 · Scene and SceneManager: switch, persistent entities

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **11 Scenes & assets** · status and owner live on the board.

## Goal

One active scene, deferred switching, persistent entities.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/scene/Scene.hpp`
- `include/engine/scene/SceneManager.hpp`
- `src/scene/SceneManager.cpp`

## Depends on

- [ENG-42](ENG-42.md) — SceneLoader: two passes, errors with file:line, save
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-55](ENG-55.md) — Unit tests: scheduler, events, input, scenes, session
- [ENG-59](ENG-59.md) — Editor (Track 1): hierarchy, inspector, scene save

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
