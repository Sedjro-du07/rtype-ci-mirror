# ENG-54 · Unit tests: ECS, reflection, commands, hooks

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **14 Tests & bench** · **Wave 6** · status and owner live on the board.

## Goal

Tests for ECS and reflection.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `tests/ecs/`
- `tests/reflection/`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `tests/`

```text
tests/ecs/        EntityAllocatorTest, SparseSetTest, RegistryTest, ViewTest, CommandsTest, HooksTest
tests/reflection/ TypeRegistryTest (each category, DuplicateName, fingerprint), BuilderCompileFailTest,
                  YamlRoundTripTest (every FieldKind), ByteStreamTest
```

## Depends on

- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy
- [ENG-20](ENG-20.md) — Commands buffer (spawn, prefab spawn, destroy, add, set, remove)
- [ENG-21](ENG-21.md) — Component hooks onCreate / onReplace / onDestroy
- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)

## Unlocks

- no task waits for this one

## Read first

- [reference 03](../engine/reference/03-ecs.md)

## To implement

- [ ] Entities, storages, registry, views, commands, hooks
- [ ] Type registration, YAML round trips

## Done when

- Coverage of every public method of blocks 3 and 4

## Watch out

- Compile-fail tests for wrong builder options

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
