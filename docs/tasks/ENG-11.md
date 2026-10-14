# ENG-11 · IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **02 Modules** · **Wave 2** · status and owner live on the board.

## Goal

The contracts between the engine, plugins and games, plus the macros that export them.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/module/Export.hpp`
- `include/engine/module/IPlugin.hpp`
- `include/engine/module/IGameModule.hpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/module/Export.hpp`

```cpp
inline constexpr uint32_t ENGINE_PLUGIN_API_VERSION = 1;  // bumped when any plugin interface changes
inline constexpr uint32_t ENGINE_GAME_API_VERSION   = 1;  // bumped when IGameModule or Context changes

#define ENGINE_EXPORT /* __declspec(dllexport) on Windows, visibility("default") elsewhere */

// Placed once at the end of the game's module .cpp. Dynamic mode: defines
//   extern "C" ENGINE_EXPORT IGameModule* create_game() noexcept;   (try/catch → nullptr)
//   extern "C" ENGINE_EXPORT void destroy_game(IGameModule*) noexcept;
//   extern "C" ENGINE_EXPORT uint32_t game_api_version() noexcept;
// Static mode: defines a registrar object that adds the class to StaticModuleTable at startup.
#define ENGINE_GAME_MODULE(ClassName)  /* … */

// Same for plugins: create_plugin / destroy_plugin / plugin_api_version, or static registration.
#define ENGINE_PLUGIN(ClassName)       /* … */
```

### `include/engine/module/IPlugin.hpp`

```cpp
enum class PluginKind : uint8_t { Platform, Audio, Network, Scripting };

struct PluginContext {          // what the engine gives a plugin at init (dependency injection)
    Logger&             log;    // tagged with the plugin name
    CVars&              cvars;  // a plugin may define its own console variables
    const EngineConfig& config;
    EventBus&           events;
};

class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual PluginKind       kind() const = 0;          // which interface the object also implements
    virtual std::string_view name() const = 0;          // "sfml", "miniaudio", "asio", "lua"
    virtual uint32_t         apiVersion() const = 0;    // must equal ENGINE_PLUGIN_API_VERSION
    virtual Result<void>     init(const PluginContext&) = 0;  // open devices, create threads
    virtual void             shutdown() noexcept = 0;   // release everything; called before destroy_plugin
};
```

### `include/engine/module/IGameModule.hpp`

```cpp
class IGameModule {
public:
    virtual ~IGameModule() = default;
    virtual void registerTypes(TypeRegistry& types) = 0;               // every type the game invents (add<T>)
    virtual void registerSystems(Role role, Scheduler& scheduler) = 0; // the game's systems for this role
    virtual std::string_view firstScene() const = 0;                   // path relative to the game root
    virtual void onStart(Context& ctx) {}                              // once, after the first scene, before the loop
    virtual void onShutdown(Context& ctx) {}                           // once, world still alive
};
```

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
