# ENG-13 · StaticModuleTable for ENGINE_DYNAMIC_MODULES=OFF

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **02 Modules** · status and owner live on the board.

## Goal

Static-link replacement for dynamic loading (`ENGINE_DYNAMIC_MODULES=OFF`).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/module/StaticModuleTable.hpp`
- `src/module/StaticModuleTable.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/module/StaticModuleTable.hpp`

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

## Depends on

- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE

## Unlocks

- [ENG-14](ENG-14.md) — ModuleManager: load by kind and name, version check, reverse unload

## Read first

- [ADR 0009](../adr/0009-dual-linking-mode.md)

## To implement

- [ ] Tables of plugin and game factories filled by registrars
- [ ] `findPlugin`, `findGame`

## Done when

- Static build loads the same fixtures as the dynamic build

## Watch out

- Static init order: use function-local statics for the tables

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
