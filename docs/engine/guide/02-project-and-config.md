# 02 · Project and configuration

## CMake

```cmake
cmake_minimum_required(VERSION 3.25)
project(mygame CXX)
find_package(engine REQUIRED)                       # or add_subdirectory(engine) inside the monorepo

engine_add_game_module(mygame
    SOURCES src/MyGameModule.cpp src/Systems.cpp)   # → games/libmygame.so (or static)

engine_add_launcher(mygame_client CONFIG client.yaml GAME mygame)
engine_add_launcher(mygame_server CONFIG server.yaml GAME mygame)
```

| Function | Does |
|---|---|
| `engine_add_game_module(name SOURCES … [DEPENDS …])` | builds the game as a module, shared or static per `ENGINE_DYNAMIC_MODULES`, output in `games/` |
| `engine_add_launcher(exe CONFIG file [GAME name])` | builds the engine's `main()` under that name, copies the config next to it |
| `engine_add_plugin(kind_name SOURCES …)` | builds a plugin (see 14) |

## The module class

```cpp
#include <engine/Context.hpp>
#include <engine/module/IGameModule.hpp>

class MyGameModule : public engine::IGameModule {
public:
    void registerTypes(engine::TypeRegistry& t) override;              // components, events, enums
    void registerSystems(engine::Role r, engine::Scheduler& s) override; // systems for this role
    std::string_view firstScene() const override { return "scenes/menu.yaml"; }
    void onStart(engine::Context& ctx) override;                       // optional
    void onShutdown(engine::Context& ctx) override;                    // optional
};
ENGINE_GAME_MODULE(MyGameModule)
```

## Configuration file

Every key, with its default. Unknown keys are an error reported with `file:line`.

```yaml
role: client                 # client | server | editor
game: mygame                 # game module name (games/libmygame.so)
platform: sfml               # sfml | raylib            (client, editor)
audio: miniaudio             #                          (client, editor)
network: asio
scripting: lua               # empty to disable scripting
tickRate: 60                 # fixed simulation steps per second (same value on client and server)

window:
  title: "My Game"
  width: 1280
  height: 720
  virtualWidth: 1280         # logical resolution the game draws in, letterboxed
  virtualHeight: 720
  fullscreen: false
  vsync: true
  uiScale: 1.0

net:
  port: 4242
  maxClients: 4
  heartbeatSeconds: 0.5
  timeoutSeconds: 5.0
  maxDatagramBytes: 1200
  snapshotRate: 20

log:
  level: info                # trace | debug | info | warn | error
  file: logs/mygame.log
  color: true

cvars:                       # any console variable, see 13
  snd_music: 0.6
  r_color_filter: none
```

## Command line

| Option | Effect |
|---|---|
| `--config <file>` | use another config file |
| `--platform <name>`, `--audio <name>`, `--network <name>`, `--scripting <name>` | override the plugin |
| `--role <client\|server\|editor>` | override the role |
| `--port <n>`, `--host <address>` | network: default port of `startServer()`, default address the game may pass to `connectTo()` |
| `--set <cvar>=<value>` | set a console variable (repeatable) |
| `--log <level>` | log level |
| `--scene <path>` | start on this scene instead of `firstScene()` (tests, debugging) |
| `--ticks <n>` | quit cleanly after `n` simulation steps (tests, CI) |
| `--platform null`, `--audio null`, `--network null`, `--scripting null` | use the built-in Null Objects (headless runs) |

## Exit codes

| Code | Meaning |
|---|---|
| 0 | normal exit |
| 1 | configuration invalid |
| 2 | plugin missing, invalid or with a different API version |
| 3 | game module missing or invalid |
| 4 | first scene could not be loaded |
| 5 | client stopped after an error in a system |
