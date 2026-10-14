# ENG-07 · EngineConfig: YAML file and command-line overrides

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **01 Core** · status and owner live on the board.

## Goal

Read the configuration file and the command line into one `EngineConfig`.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/EngineConfig.hpp`
- `src/core/EngineConfig.cpp`

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
