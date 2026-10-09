# `engine/include/engine/module/`

Plugin and game interfaces, loading, Null Objects.

Placeholder created from the engine architecture (2026-10-09). Replace this file's list as files are written.

| File to create | Task |
|---|---|
| `DynamicLibrary.hpp` | ENG-12 — DynamicLibrary for Linux and Windows |
| `Export.hpp` | ENG-11 — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE |
| `IGameModule.hpp` | ENG-11 — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE |
| `IPlugin.hpp` | ENG-11 — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE |
| `ModuleManager.hpp` | ENG-14 — ModuleManager: load by kind and name, version check, reverse unload |
| `NullPlugins.hpp` | ENG-15 — Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting |
| `StaticModuleTable.hpp` | ENG-13 — StaticModuleTable for ENGINE_DYNAMIC_MODULES=OFF |

Design: `docs/engine/reference/`. Rules: `docs/engine/folder-tree.md`.
