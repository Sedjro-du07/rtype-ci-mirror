# ENG-53 · Unit tests: core and modules, with a test plugin and a test game

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **14 Tests & bench** · **Wave 5** · status and owner live on the board.

## Goal

Test setup and tests for core and modules, with a test plugin and a test game.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `tests/CMakeLists.txt`
- `tests/core/`
- `tests/module/`
- `tests/fixtures/`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `tests/`

```text
tests/CMakeLists.txt          engine_add_test(<name> SOURCES ...) helper, test framework (chosen in Tooling)
tests/fixtures/TestPlugin.cpp  class TestPlatform : public IPlatform { ... };  ENGINE_PLUGIN(TestPlatform)
tests/fixtures/TestGame.cpp    class TestGame : public IGameModule { ... };    ENGINE_GAME_MODULE(TestGame)
tests/core/   HashTest, MathTest, ErrorTest, TimeTest, SignalTest, EngineConfigTest
tests/module/ DynamicLibraryTest, StaticModuleTableTest, ModuleManagerTest (version mismatch, wrong kind, null, unload order)
```

## Depends on

- [ENG-14](ENG-14.md) — ModuleManager: load by kind and name, version check, reverse unload

## Unlocks

- no task waits for this one

## Read first

- [reference 01](../engine/reference/01-core.md)

## To implement

- [ ] `tests/CMakeLists.txt` with the chosen framework
- [ ] Fixtures: test plugin, test game
- [ ] Tests of ENG-04..15

## Done when

- `ctest` runs green on Linux and Windows

## Watch out

- Fixtures must build in both linking modes

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
