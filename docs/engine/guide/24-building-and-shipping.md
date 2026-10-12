# 24 · Building and shipping

## Two build modes

| | Dynamic (default) | Static |
|---|---|---|
| Option | `-DENGINE_DYNAMIC_MODULES=ON` | `-DENGINE_DYNAMIC_MODULES=OFF` |
| Plugins and game | `.so` / `.dll` loaded at runtime | linked into each executable |
| Swap SFML ↔ raylib | change one config line | rebuild |
| Files to ship | executable, `libengine`, `plugins/`, `games/`, content | executable + content |
| Use for | development, Track 1 demo, editor | release, and a safety net before a defense |

Game code is identical in both modes (`ENGINE_GAME_MODULE` and `engine_add_game_module` adapt).

## Build types

```bash
cmake -B build-debug   -DCMAKE_BUILD_TYPE=Debug          # asserts, debug symbols for every module
cmake -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j
```

Every module (engine, plugins, game) must use the same build type and compiler: mixing a Debug
plugin with a Release engine breaks the binary contract.

## What to ship

```
r-type/
├── r-type_client(.exe)  r-type_server(.exe)
├── client.yaml  server.yaml  input.yaml
├── libengine.so | engine.dll                       (dynamic mode)
├── plugins/  games/                                (dynamic mode)
├── scenes/  prefabs/  scripts/  assets/
└── (Windows) the C++ runtime DLLs next to the executables
```

Do not ship `src/`, build folders, or `user/` (created at first run).

## Windows notes

- DLLs are looked up next to the executable; keep `plugins/` and `games/` beside it.
- Build engine, plugins and game with the same Visual Studio version and runtime (`/MD`).

## Checklist before a defense

- [ ] Release build of client and server on Linux **and** Windows.
- [ ] Static build available as a fallback.
- [ ] Headless run of N ticks passes (page 23).
- [ ] Config files point to the right game, plugins and port.
- [ ] Debug overlay and console work (F1, F2) to show metrics live.
