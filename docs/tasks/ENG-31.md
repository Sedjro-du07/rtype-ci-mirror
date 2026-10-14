# ENG-31 · platform_sfml plugin: window, events to InputMap, batching renderer

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **08 Rendering** · status and owner live on the board.

## Goal

The Part 1 window, input and renderer on SFML 3.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/platform_sfml/CMakeLists.txt`
- `plugins/platform_sfml/SfmlPlatform.hpp`
- `plugins/platform_sfml/SfmlPlatform.cpp`
- `plugins/platform_sfml/SfmlRenderer.hpp`
- `plugins/platform_sfml/SfmlRenderer.cpp`
- `plugins/platform_sfml/SfmlKeys.cpp`

## Depends on

- [ENG-30](ENG-30.md) — Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform
- [ENG-29](ENG-29.md) — InputMap and ActionState: input.yaml, rebinding, accessibility options, masks
- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE
- [ENG-02](ENG-02.md) — CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher

## Unlocks

- [ENG-32](ENG-32.md) — Dear ImGui and ImPlot in platform_sfml (debug UI hooks)
- [ENG-33](ENG-33.md) — Colour-blindness filters and UI scale in platform_sfml
- [ENG-58](ENG-58.md) — platform_raylib plugin: 2D, then optional 3D

## Read first

- [ADR 0007](../adr/0007-sfml-and-raylib-backends.md)
- [ADR 0031](../adr/0031-render-api.md)

## To implement

- [ ] `SfmlPlatform`: window, events → RawInputEvent → InputMap::feed, window events → EventBus
- [ ] `SfmlRenderer`: textures, fonts, sort by layer, batch by texture with VertexArray, letterbox
- [ ] SFML key mapping

## Done when

- A sprite, a text and shapes appear; resizing keeps the virtual resolution; input reaches actions

## Watch out

- One draw call per texture and layer, not per sprite

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
