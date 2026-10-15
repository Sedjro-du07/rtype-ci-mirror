# 01 · Getting started

## Prerequisites

| Platform | Compiler (C++23, ADR 0017) | Tools |
|---|---|---|
| Linux | GCC 14+ or Clang 18+ | CMake 3.25+, Ninja, [vcpkg](https://vcpkg.io) with `VCPKG_ROOT` set |
| Windows | Visual Studio 2022 (MSVC 17.10+) | same (CMake and Ninja ship with Visual Studio) |

Third-party libraries (SFML 3, raylib, miniaudio, standalone Asio, yaml-cpp, Lua 5.4, sol2,
Dear ImGui, ImPlot, Catch2) come from vcpkg in manifest mode (`vcpkg.json` at the repository root,
ADR 0045). Nothing is installed system-wide. macOS is best effort; mobile is designed for, not built
(ADR 0044).

## Build

```bash
cmake --preset linux-gcc-debug                  # or linux-clang-debug, windows-msvc-debug, …-release, …-static
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug
```

Installing vcpkg and the compilers, and every other command: [29 · Every command](29-commands.md).
The presets (`CMakePresets.json`) are the only supported way to configure; VS Code, CLion and Visual
Studio list them too. `-static` presets build one executable with no `.so` / `.dll`.

Output layout (`build/<preset>/bin/`):

```
build/linux-gcc-debug/bin/
├── r-type_client            launchers (the engine's main())
├── r-type_server
├── client.yaml  server.yaml configuration copied next to them
├── libengine.so             the engine (static mode: inside the executables)
├── plugins/                 libplatform_sfml.so, libaudio_miniaudio.so, libnetwork_asio.so, libscripting_lua.so
├── games/                   librtype_server.so, librtype_client.so (ADR 0040)
└── assets/ scenes/ prefabs/ scripts/   copied from the game folder
```

## Run

```bash
./build/bin/r-type_server                       # headless; the game calls startServer() → port from server.yaml
./build/bin/r-type_client                       # opens the window
./build/bin/r-type_client --platform raylib     # same game on another backend
./build/bin/r-type_client --set r_debug_colliders=1
```

## What a game is made of

```
mygame/
├── CMakeLists.txt     engine_add_game_module + engine_add_launcher
├── client.yaml        configuration of the client launcher
├── server.yaml        configuration of the server launcher (if networked)
├── input.yaml         actions and their default bindings
├── src/               C++ compiled into the game module: components, systems, the IGameModule class
├── scenes/            YAML: what exists at the start of each level or screen
├── prefabs/           YAML: reusable entity templates
├── scripts/           Lua behaviours
└── assets/            images, sounds, music, fonts
```

Everything outside `src/` is read at launch: edit it and relaunch, no recompilation.

## Next

Follow the [Pong tutorial](../making-a-game.md), then read the topic pages.
