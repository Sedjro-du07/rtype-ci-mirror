# ENG-22 · TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **04 Reflection** · status and owner live on the board.

## Goal

Teach the engine the types a game invents: names, kinds, fields, options (ADR 0027, 0036).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/reflection/TypeKey.hpp`
- `include/engine/reflection/TypeInfo.hpp`
- `include/engine/reflection/TypeBuilder.hpp`
- `include/engine/reflection/TypeRegistry.hpp`
- `src/reflection/TypeRegistry.cpp`

## Depends on

- [ENG-17](ENG-17.md) — IStorage and paged SparseSet<T>
- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError

## Unlocks

- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy
- [ENG-23](ENG-23.md) — YAML wrappers and field codecs
- [ENG-25](ENG-25.md) — Engine components and their registration
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events
- [ENG-39](ENG-39.md) — Replication: NetworkIdMap, net components, snapshot and input systems
- [ENG-54](ENG-54.md) — Unit tests: ECS, reflection, commands, hooks
- [SRV-02](SRV-02.md) — Handshake: player name, format fingerprint, ServerFull / FormatMismatch, TCP ↔ UDP link
- [SRV-04](SRV-04.md) — Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits

## Read first

- [ADR 0027](../adr/0027-reflection-by-name.md)
- [ADR 0036](../adr/0036-one-add-and-type-categories.md)
- [reference 04](../engine/reference/04-reflection.md)

## To implement

- [ ] `TypeCategory`, `TypeName<T>`, `HasTypeName`, `typeKey<T>`
- [ ] Macros `ENGINE_COMPONENT`, `ENGINE_EVENT`, `ENGINE_MESSAGE`, `ENGINE_STRUCT`, `ENGINE_ENUM` (qualified name → last part)
- [ ] `FieldKind` (with List, Struct), `FieldInfo` (maxLength, element kind), `TypeFlags`, `Transport`, `TypeInfo`
- [ ] `FieldsBuilder<T>` (`field` with deducing this, `maxLength`) and the five builders with their options
- [ ] `TypeRegistry::add<T>()` choosing the builder with `if constexpr`; `id`, `find`, `findByKey`, `info`, `all`, `messages`, `formatFingerprint`, `closestName`, `clearGameTypes`

## Done when

- Tests: each category registers; wrong option fails to compile (compile-fail test); DuplicateName; fingerprint changes when a field changes

## Watch out

- `static_assert` message must name the five macros
- Engine types register before the game's

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
