# ENG-49 · scripting_lua plugin: sol2 API, sandbox, reload

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **12 Services** · status and owner live on the board.

## Goal

Lua 5.4 + sol2 implementation of IScripting.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/scripting_lua/CMakeLists.txt`
- `plugins/scripting_lua/LuaScripting.hpp`
- `plugins/scripting_lua/LuaScripting.cpp`
- `plugins/scripting_lua/LuaApi.cpp`
- `plugins/scripting_lua/Sandbox.cpp`

## Depends on

- [ENG-48](ENG-48.md) — IScripting, Script component, script systems
- [ENG-02](ENG-02.md) — CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher

## Unlocks

- no task waits for this one

## Read first

- [ADR 0019](../adr/0019-lua-first-custom-language-optional.md)
- [guide 11](../engine/guide/11-scripting-lua.md)

## To implement

- [ ] API of guide 11 by reflection (components by name)
- [ ] Sandbox (no io, os, package, debug, load)
- [ ] Instruction limit hook
- [ ] Reload keeping the handle

## Done when

- The guide 11 boss script runs; an infinite loop is stopped

## Watch out

- No binding code per component type

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
