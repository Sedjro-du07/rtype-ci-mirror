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

## Build types and presets

```bash
cmake --preset linux-gcc-debug       # asserts, debug symbols for every module
cmake --preset linux-gcc-release
cmake --build --preset linux-gcc-release
```

| Preset | Platform | Mode |
|---|---|---|
| `linux-gcc-debug` / `-release` | Linux, GCC 14 | dynamic |
| `linux-clang-debug` / `-release` | Linux, Clang 18 | dynamic |
| `windows-msvc-debug` / `-release` | Windows, MSVC | dynamic |
| `linux-gcc-static`, `windows-msvc-static` | Linux, Windows | static |

Every preset uses the vcpkg toolchain from `VCPKG_ROOT` (ADR 0045).

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
- The engine's public classes are marked `ENGINE_API` (`engine/Export.hpp`, generated). Libraries
  build with hidden visibility on Linux too, so a missing mark fails on both systems (ADR 0044).
- Asset paths must match the file's case exactly; Windows is checked like Linux.

## Packages and installing the engine

- `cmake --install build/<preset> --prefix <dir>` installs headers, libraries, plugins and
  `rtype_engineConfig.cmake`: another project uses `find_package(rtype_engine)` (task ENG-62).
- `cpack --preset <preset>` builds `r-type-<version>-Linux.tar.gz` or `…-Windows.zip` with the layout
  below and the runtime DLLs.

## CI

Every push and pull request builds GCC, Clang and MSVC in dynamic mode, GCC and MSVC in static mode,
runs the tests and the headless smoke test, and checks formatting (`.github/workflows/ci.yml`,
task ENG-61). A red job blocks the merge into `engine` and `develop`.

## Mobile (designed for, not built)

Static mode is the mobile build (iOS forbids loading code at runtime). A port is a platform plugin
(`platform_raylib` runs on Android), touch events and `AppSuspended` / `AppResumed` are reserved,
and the engine never writes under the game root (ADR 0044).

## Checklist before a defense

- [ ] Release build of client and server on Linux **and** Windows.
- [ ] Static build available as a fallback.
- [ ] Headless run of N ticks passes (page 23).
- [ ] Config files point to the right game, plugins and port.
- [ ] Debug overlay and console work (F1, F2) to show metrics live.
