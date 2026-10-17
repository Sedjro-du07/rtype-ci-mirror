# ENG-14 · ModuleManager: load by kind and name, version check, reverse unload

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **02 Modules** · **Wave 4** · status and owner live on the board.

## Goal

Load plugins by kind and name and the game module, check versions, unload in reverse order.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/module/ModuleManager.hpp`
- `src/module/ModuleManager.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/module/ModuleManager.hpp`

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
    Result<IGameModule*> loadGame(std::string_view name, Role role); // lib<name>_<role> first, then lib<name>;
                                                              // checks game_api_version, calls create_game
    void unloadGame();                                        // destroy_game, then close the library
    void unloadAll() noexcept;                                // reverse load order: shutdown, destroy, close
    std::span<const LoadedModule> loaded() const;             // for the "modules" console command
};
```

## Depends on

- [ENG-12](ENG-12.md) — DynamicLibrary for Linux and Windows
- [ENG-13](ENG-13.md) — StaticModuleTable for ENGINE_DYNAMIC_MODULES=OFF
- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE
- [ENG-44](ENG-44.md) — Logger and sinks (console, file, developer console)

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-53](ENG-53.md) — Unit tests: core and modules, with a test plugin and a test game

## Read first

- [ADR 0025](../adr/0025-module-loading.md)
- [reference 02](../engine/reference/02-modules.md)

## To implement

- [ ] `loadPlugin<I>(kind, name, ctx)`: `null` → built-in Null Object; else library + version + kind check + `init`
- [ ] `loadGame(name, role)`: `lib<name>_<role>` first, then `lib<name>` (ADR 0040)
- [ ] `unloadGame`, `unloadAll` in reverse load order
- [ ] `loaded()` for the console

## Done when

- Tests: version mismatch → ApiMismatch, wrong kind, null plugin, unload order logged

## Watch out

- A client whose platform plugin fails stops with code 2: never fall back to NullPlatform

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
