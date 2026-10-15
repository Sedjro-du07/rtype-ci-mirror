# ENG-07 · EngineConfig: YAML file and command-line overrides

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **01 Core** · **Wave 5** · status and owner live on the board.

## Goal

Read the configuration file and the command line into one `EngineConfig`.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/EngineConfig.hpp`
- `src/core/EngineConfig.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/EngineConfig.hpp`

```cpp
enum class Role : uint8_t { Client, Server, Editor };

struct EngineConfig {
    Role                  role = Role::Client;
    std::string           game;             // game module name: "rtype" → games/librtype_<role>.so (ADR 0040)
    std::string           platform = "sfml";// platform plugin (client, editor)
    std::string           audio = "miniaudio";
    std::string           network = "asio";
    std::string           scripting = "lua";// empty string = no scripting plugin (NullScripting)
    double                tickRate = 60.0;  // fixed simulation steps per second (ADR 0003)
    std::filesystem::path root;             // folder of the game: assets/, scenes/, prefabs/
    WindowDesc            window;           // client only, see block 8
    NetConfig             net;              // see block 10
    LogConfig             log;              // see block 12
    YAML::Node            cvars;            // initial console variables, see block 12
    std::filesystem::path sceneOverride;    // --scene: start here instead of IGameModule::firstScene()
    uint64_t              maxTicks = 0;     // --ticks: quit cleanly after N steps (0 = never); tests and CI
    RoomsConfig           rooms;            // server: host ("shared"), maxRooms (8), workerThreads (0 = cores); reference 14

    // Reads the YAML file, then applies command-line overrides (--config, --role, --platform, --audio,
    // --network, --scripting, --port, --host, --log, --scene, --ticks, --set name=value). Unknown keys are an error with file:line.
    static Result<EngineConfig> load(const std::filesystem::path& file, int argc, char** argv);
};

struct RoomsConfig {                      // server.yaml "rooms:" (ADR 0038)
    std::string host = "shared";          // "dedicated" in Part 2
    uint32_t    maxRooms = 8;
    uint32_t    workerThreads = 0;        // 0 = number of cores
};
```

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError
- [ENG-23](ENG-23.md) — YAML wrappers and field codecs

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy

## Read first

- [ADR 0021](../adr/0021-engine-composition-root.md)
- [guide 02](../engine/guide/02-project-and-config.md)

## To implement

- [ ] YAML keys of `EngineConfig` (role, game, plugins, tickRate, root, window, net, log, cvars)
- [ ] `rooms` section (host, maxRooms, workerThreads)
- [ ] CLI options: --config, --role, --platform, --audio, --network, --scripting, --port, --host, --log, --scene, --ticks, --set name=value
- [ ] Unknown key → error with `file:line`; CLI overrides the file

## Done when

- Tests: valid file, unknown key, wrong type, every CLI option overriding the file

## Watch out

- `YAML::Node` stays in `EngineConfig.hpp`, which games never include

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
