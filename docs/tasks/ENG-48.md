# ENG-48 · IScripting, Script component, script systems

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **12 Services** · status and owner live on the board.

## Goal

The scripting interface, the Script component and the script systems.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/script/IScripting.hpp`
- `src/script/ScriptSystems.cpp`

## Depends on

- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events
- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE

## Unlocks

- [ENG-15](ENG-15.md) — Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting
- [ENG-40](ENG-40.md) — AssetManager: handles, cache, refcount, fallback, hot reload
- [ENG-49](ENG-49.md) — scripting_lua plugin: sol2 API, sandbox, reload

## Read first

- [ADR 0019](../adr/0019-lua-first-custom-language-optional.md)
- [ADR 0035](../adr/0035-services-and-tools.md)

## To implement

- [ ] `ScriptHost`, `IScripting` declarations
- [ ] Script systems: lifecycle, update, collision; error → script disabled

## Done when

- With NullScripting, a Script component logs one clear error

## Watch out

- Script errors never stop the game

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
