# 2 · Modules and plugins

Status: **Locked**. ADRs 0002, 0004, 0005, 0009, 0020, 0023, 0025.
Folder: `engine/include/engine/module/`, `engine/src/module/`, `engine/cmake/EngineModules.cmake`.

## Role

Load backends (plugins) and the game (game module) at runtime, or from a static table when
`ENGINE_DYNAMIC_MODULES=OFF`, with one code path for both. Unload them last, in reverse order.

---

## Versions and export macros — `module/Export.hpp`

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

## `enum class PluginKind` and `struct PluginContext` — `module/IPlugin.hpp`

```cpp
enum class PluginKind : uint8_t { Platform, Audio, Network, Scripting };

struct PluginContext {          // what the engine gives a plugin at init (dependency injection)
    Logger&             log;    // tagged with the plugin name
    CVars&              cvars;  // a plugin may define its own console variables
    const EngineConfig& config;
    EventBus&           events;
};
```

## `class IPlugin` — `module/IPlugin.hpp`

```cpp
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

`IPlatform`, `IAudio`, `INetwork` and `IScripting` derive from `IPlugin` (blocks 8, 9, 10, 12).

## `class IGameModule` — `module/IGameModule.hpp`

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

## `class DynamicLibrary` — `module/DynamicLibrary.hpp`

The only OS-specific loading code (Adapter over `dlopen` / `LoadLibraryW`). RAII, move-only.

```cpp
class DynamicLibrary {
public:
    static Result<DynamicLibrary> open(const std::filesystem::path& file); // FileNotFound / SymbolNotFound with the OS message
    static std::string decorate(std::string_view stem);  // "platform_sfml" → "libplatform_sfml.so" / "platform_sfml.dll"
    DynamicLibrary(DynamicLibrary&&) noexcept; DynamicLibrary& operator=(DynamicLibrary&&) noexcept;
    ~DynamicLibrary();                                   // dlclose / FreeLibrary
    void* rawSymbol(std::string_view name) const;        // nullptr if absent
    template <class Fn> Result<Fn> symbol(std::string_view name) const; // typed lookup, SymbolNotFound if absent
    const std::filesystem::path& path() const;
};
```

## `class StaticModuleTable` — `module/StaticModuleTable.hpp`

Used only when `ENGINE_DYNAMIC_MODULES=OFF` (ADR 0009). Filled by the registrars the macros create.

```cpp
class StaticModuleTable {
public:
    using CreatePlugin = IPlugin* (*)() noexcept;   using DestroyPlugin = void (*)(IPlugin*) noexcept;
    using CreateGame   = IGameModule* (*)() noexcept; using DestroyGame = void (*)(IGameModule*) noexcept;
    static void addPlugin(std::string_view stem, CreatePlugin, DestroyPlugin);  // "platform_sfml"
    static void addGame(std::string_view name, CreateGame, DestroyGame);        // "rtype"
    static const PluginEntry* findPlugin(std::string_view stem);
    static const GameEntry*   findGame(std::string_view name);
};
```

## `class ModuleManager` — `module/ModuleManager.hpp`

```cpp
struct LoadedModule {                 // one row per loaded plugin or game, for logs and the console
    std::string           stem;       // "platform_sfml", "rtype"
    std::filesystem::path file;       // empty in static mode
    bool                  isGame;
};

class ModuleManager {
public:
    ModuleManager(Logger& log, std::filesystem::path executableDir); // searches <dir>/plugins and <dir>/games
    ~ModuleManager();                                                 // unloadAll()

    // Loads "<kind>_<name>" (e.g. platform_sfml), checks apiVersion and kind, calls init().
    template <class I> Result<I*> loadPlugin(PluginKind kind, std::string_view name, const PluginContext& ctx);
    Result<IGameModule*> loadGame(std::string_view name);     // checks game_api_version, calls create_game
    void unloadGame();                                        // destroy_game, then close the library
    void unloadAll() noexcept;                                // reverse load order: shutdown, destroy, close
    std::span<const LoadedModule> loaded() const;             // for the "modules" console command
};
```

Rules enforced here: version mismatch → `ApiMismatch`; `create_*` returning `nullptr` → `GameModuleInvalid`
or `PluginNotFound`; every object is destroyed by the `destroy_*` of the module that created it.

## Null Objects — `module/NullPlugins.hpp`

| Class | Implements | Behaviour |
|---|---|---|
| `NullPlatform` | `IPlatform` | no window; `pollEvents` produces nothing; `renderer()` returns `NullRenderer` |
| `NullRenderer` | `IRenderer` | `createTexture` returns a valid handle without decoding; `submit` ignores |
| `NullAudio` | `IAudio` | every call does nothing; `play` returns an invalid voice id |
| `NullNetwork` | `INetwork` | for offline runs and tests; `send` drops, `receive` yields nothing |
| `NullScripting` | `IScripting` | used when `scripting` is empty; `load` returns an error explaining scripting is disabled |

Used on the server, in headless CI, and when no audio device exists (ADR 0023). The plugin name `null`
(`platform: null`, `--audio null`…) selects the built-in Null Object of that kind instead of loading a library. A client whose
platform plugin fails to load stops with exit code 2; it never falls back to `NullPlatform`.
Each Null Object in use is logged once at boot.

---

## CMake functions — `engine/cmake/EngineModules.cmake`

```cmake
option(ENGINE_DYNAMIC_MODULES "Build plugins and games as runtime-loaded libraries" ON)

engine_add_plugin(<kind>_<name> SOURCES ... [DEPENDS <targets>])
# SHARED or STATIC per ENGINE_DYNAMIC_MODULES; links engine; output to <bin>/plugins;
# hidden visibility; defines ENGINE_BUILDING_PLUGIN.

engine_add_game_module(<name> SOURCES ... [DEPENDS <targets>])
# Same for a game; output to <bin>/games.

engine_add_launcher(<executable> CONFIG <file.yaml> [GAME <name>])
# Builds engine/launcher/main.cpp with ENGINE_DEFAULT_CONFIG="<file.yaml>"; copies the config next
# to the executable; in static mode links every plugin and the named game into the executable.
```
