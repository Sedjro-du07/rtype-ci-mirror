# 01 · Getting started

## Prerequisites

| Platform | Compiler (C++23, ADR 0017) | Tools |
|---|---|---|
| Linux | GCC 14+ or Clang 18+ | CMake 3.25+, the package manager chosen in the Tooling part |
| Windows | Visual Studio 2022 (MSVC 17.10+) | same |

Third-party libraries (SFML 3, raylib, miniaudio, standalone Asio, yaml-cpp, Lua 5.4, sol2,
Dear ImGui, ImPlot) are fetched by the package manager. Nothing is installed system-wide.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release           # add -DENGINE_DYNAMIC_MODULES=OFF for one static binary
cmake --build build -j
```

Output layout:

```
build/bin/
├── r-type_client            launchers (the engine's main())
├── r-type_server
├── client.yaml  server.yaml configuration copied next to them
├── libengine.so             the engine (static mode: inside the executables)
├── plugins/                 libplatform_sfml.so, libaudio_miniaudio.so, libnetwork_asio.so, libscripting_lua.so
├── games/                   librtype.so
└── assets/ scenes/ prefabs/ scripts/   copied from the game folder
```

## Run

```bash
./build/bin/r-type_server                       # headless, listens on the port from server.yaml
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
