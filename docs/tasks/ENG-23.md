# ENG-23 · YAML wrappers and field codecs

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **04 Reflection** · status and owner live on the board.

## Goal

YAML reading and writing for any registered type, without yaml-cpp in game headers.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/reflection/Yaml.hpp`
- `src/reflection/Yaml.cpp`

## Depends on

- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)

## Unlocks

- [ENG-07](ENG-07.md) — EngineConfig: YAML file and command-line overrides
- [ENG-29](ENG-29.md) — InputMap and ActionState: input.yaml, rebinding, accessibility options, masks
- [ENG-41](ENG-41.md) — PrefabLibrary and instantiate
- [ENG-45](ENG-45.md) — CVars: typed variables, flags, ranges, archive

## Read first

- [ADR 0018](../adr/0018-data-in-yaml-behaviour-in-scripts.md)
- [ADR 0027](../adr/0027-reflection-by-name.md)

## To implement

- [ ] `YamlNode`, `YamlWriter`, `LoadContext` / `SaveContext` (Entity ↔ Uuid)
- [ ] Generic YAML codec per FieldKind: lists as sequences, structs as maps, enums by name

## Done when

- Tests: round trip of every field kind; error with file:line on wrong type

## Watch out

- Unknown key in a component → error with closest name

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
