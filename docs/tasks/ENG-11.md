# ENG-11 · IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **02 Modules** · status and owner live on the board.

## Goal

The contracts between the engine, plugins and games, plus the macros that export them.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/module/Export.hpp`
- `include/engine/module/IPlugin.hpp`
- `include/engine/module/IGameModule.hpp`

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError

## Unlocks

- [ENG-13](ENG-13.md) — StaticModuleTable for ENGINE_DYNAMIC_MODULES=OFF
- [ENG-14](ENG-14.md) — ModuleManager: load by kind and name, version check, reverse unload
- [ENG-15](ENG-15.md) — Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting
- [ENG-30](ENG-30.md) — Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform
- [ENG-31](ENG-31.md) — platform_sfml plugin: window, events to InputMap, batching renderer
- [ENG-34](ENG-34.md) — IAudio, PlayParams, buses, captions
- [ENG-36](ENG-36.md) — INetwork and network types
- [ENG-48](ENG-48.md) — IScripting, Script component, script systems

## Read first

- [ADR 0020](../adr/0020-game-module-helpers.md)
- [ADR 0025](../adr/0025-module-loading.md)
- [reference 02](../engine/reference/02-modules.md)

## To implement

- [ ] `IPlugin`, `PluginKind`, `PluginContext`
- [ ] `IGameModule` with default `onStart` / `onShutdown`
- [ ] `ENGINE_PLUGIN_API_VERSION`, `ENGINE_GAME_API_VERSION`, `ENGINE_EXPORT`
- [ ] `ENGINE_GAME_MODULE(C)` and `ENGINE_PLUGIN(C)`: extern "C" create / destroy / version, try/catch → nullptr; static registrar in static mode

## Done when

- Test fixture plugin and game build and are found by ModuleManager in both modes

## Watch out

- No exception may leave an extern "C" function
- Objects are destroyed by the module that created them

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
