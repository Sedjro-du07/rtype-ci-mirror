# ENG-30 · Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **08 Rendering** · status and owner live on the board.

## Goal

The rendering API as plain data: the engine never draws itself.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/render/Handles.hpp`
- `include/engine/render/WindowDesc.hpp`
- `include/engine/render/Camera2D.hpp`
- `include/engine/render/DrawCmd.hpp`
- `include/engine/render/RenderQueue.hpp`
- `include/engine/render/IRenderer.hpp`
- `include/engine/render/IPlatform.hpp`
- `src/render/RenderQueue.cpp`
- `src/render/Camera2D.cpp`

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError
- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-15](ENG-15.md) — Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting
- [ENG-25](ENG-25.md) — Engine components and their registration
- [ENG-31](ENG-31.md) — platform_sfml plugin: window, events to InputMap, batching renderer
- [ENG-40](ENG-40.md) — AssetManager: handles, cache, refcount, fallback, hot reload
- [ENG-52](ENG-52.md) — Render systems: sprites (interpolated), text, animation, debug colliders

## Read first

- [ADR 0006](../adr/0006-command-list-renderer.md)
- [ADR 0031](../adr/0031-render-api.md)
- [reference 08](../engine/reference/08-rendering.md)

## To implement

- [ ] Handles, WindowDesc, Feature, ColorFilter, Camera2D (screen ↔ world with letterbox)
- [ ] DrawCmd, RenderFrame, RenderStats
- [ ] RenderQueue (text arena)
- [ ] IRenderer, IPlatform declarations

## Done when

- Tests: Camera2D conversions round trip; RenderQueue keeps capacity after clear

## Watch out

- No SFML or raylib type in these headers

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
