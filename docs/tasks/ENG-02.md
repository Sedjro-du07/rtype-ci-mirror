# ENG-02 · CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **00 Build** · **Wave 2** · status and owner live on the board.

## Goal

Give games and plugins one-line CMake functions so nobody copies build boilerplate.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `cmake/EngineModules.cmake`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `cmake/EngineModules.cmake`

```cmake
option(ENGINE_DYNAMIC_MODULES "Build plugins and games as runtime-loaded libraries" ON)

engine_add_plugin(<kind>_<name> SOURCES ... [DEPENDS <targets>])
# SHARED or STATIC per ENGINE_DYNAMIC_MODULES; links engine; output to <bin>/plugins;
# hidden visibility; defines ENGINE_BUILDING_PLUGIN.

engine_add_game_module(<name> [SHARED_SOURCES ...] [SERVER_SOURCES ...] [CLIENT_SOURCES ...]
                       [SOURCES ...] [DEPENDS <targets>])
# ADR 0040. With SERVER_SOURCES / CLIENT_SOURCES: builds lib<name>_server (shared + server) and
# lib<name>_client (shared + client) in <bin>/games. With SOURCES only: one lib<name> for every role.
# Defines ENGINE_GAME_SERVER or ENGINE_GAME_CLIENT in the role builds. CMake targets are named
# game_<name>_server / game_<name>_client (or game_<name>), so a launcher may be called <name>_client.

engine_add_launcher(<executable> CONFIG <file.yaml> [GAME <name>] [ROLE client|server|editor])
# Builds the launcher main() (ENGINE_LAUNCHER_SOURCE, default engine/launcher/main.cpp) with
# ENGINE_DEFAULT_CONFIG="<file.yaml>" into <bin>; copies the config next to it. Dynamic mode: builds
# the plugins and the game with it. Static mode: links every plugin and the game whole
# (WHOLE_ARCHIVE), so their registrars run; ROLE picks lib<name>_server (server) or lib<name>_client
# (client, editor) for a split game and is required there.
# Declare plugins and games before the launchers that use them.
```

## Depends on

- [ENG-01](ENG-01.md) — Engine CMake project, vcpkg manifest, presets, options, exports and C++23 setup

## Unlocks

- [ENG-10](ENG-10.md) — Generic launcher main() and signal handling
- [ENG-31](ENG-31.md) — platform_sfml plugin: window, events to InputMap, batching renderer
- [ENG-35](ENG-35.md) — audio_miniaudio plugin
- [ENG-37](ENG-37.md) — network_asio plugin: UDP and TCP on an I/O thread
- [ENG-49](ENG-49.md) — scripting_lua plugin: sol2 API, sandbox, reload
- [ENG-62](ENG-62.md) — Installable engine: install, export, find_package(rtype_engine), CPack packages

## Read first

- [ADR 0005](../adr/0005-game-as-a-module.md)
- [ADR 0009](../adr/0009-dual-linking-mode.md)
- [ADR 0020](../adr/0020-game-module-helpers.md)

## To implement

- [ ] `engine_add_plugin(<kind>_<name> SOURCES … [DEPENDS …])`: SHARED or STATIC per option, output to `<bin>/plugins`, hidden visibility, `ENGINE_BUILDING_PLUGIN`
- [ ] `engine_add_game_module(<name> SHARED_SOURCES … SERVER_SOURCES … CLIENT_SOURCES …)`: builds `lib<name>_server` and `lib<name>_client` in `<bin>/games` (or one `lib<name>` with `SOURCES`), defines `ENGINE_GAME_SERVER` / `ENGINE_GAME_CLIENT` (ADR 0040)
- [ ] `engine_add_launcher(<exe> CONFIG <file> [GAME <name>] [ROLE client|server|editor])`: builds `launcher/main.cpp` (`ENGINE_LAUNCHER_SOURCE`) with `ENGINE_DEFAULT_CONFIG` into `bin/`, copies the config, links plugins and the role's game build whole in static mode
- [ ] Game targets named `game_<name>_server` / `game_<name>_client` (files keep `lib<name>_<role>`), so launchers may be called `<name>_client`

## Done when

- A test plugin and a test game (ENG-53 fixtures) build in both modes
- `ENGINE_DYNAMIC_MODULES=OFF` produces one executable with no `.so` / `.dll` next to it

## Watch out

- Windows: `.dll` must land next to the executable or in `plugins/` with the search path set
- Static mode needs `WHOLE_ARCHIVE` (or `/WHOLEARCHIVE`) so registrar objects are not dropped

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
