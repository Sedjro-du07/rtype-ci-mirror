# ENG-15 · Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **02 Modules** · status and owner live on the board.

## Goal

Do-nothing implementations used on the server, in CI and when a device is missing (ADR 0023).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/module/NullPlugins.hpp`
- `src/module/NullPlugins.cpp`

## Depends on

- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE
- [ENG-30](ENG-30.md) — Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform
- [ENG-34](ENG-34.md) — IAudio, PlayParams, buses, captions
- [ENG-36](ENG-36.md) — INetwork and network types
- [ENG-48](ENG-48.md) — IScripting, Script component, script systems

## Unlocks

- [ENG-57](ENG-57.md) — Headless smoke test in CI: client with Null plugins runs 1000 ticks

## Read first

- [ADR 0023](../adr/0023-null-object-services.md)

## To implement

- [ ] NullPlatform, NullRenderer (valid handles, no decoding), NullAudio, NullNetwork, NullScripting

## Done when

- Headless smoke test (ENG-57) runs with all Null Objects

## Watch out

- Each Null Object in use is logged once at boot

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
