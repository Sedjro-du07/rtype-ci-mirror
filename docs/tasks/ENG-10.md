# ENG-10 · Generic launcher main() and signal handling

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **01 Core** · **Wave 11** · status and owner live on the board.

## Goal

The engine's `main()`, shared by every executable.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `launcher/main.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `launcher/main.cpp`

```cpp
int main(int argc, char** argv);
// Loads EngineConfig from ENGINE_DEFAULT_CONFIG (set by engine_add_launcher) and argv,
// installs os::onQuitSignal (SIGINT / SIGTERM, Windows console handler) to call Engine::requestQuit,
// then returns Engine::run(). Prints Error::toString() and returns 1 if the config is invalid.
```

## Depends on

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-02](ENG-02.md) — CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher
- [ENG-60](ENG-60.md) — OS layer: secure random, user data folder, executable folder, quit signals

## Unlocks

- no task waits for this one

## Read first

- [ADR 0005](../adr/0005-game-as-a-module.md)
- [reference 01](../engine/reference/01-core.md)

## To implement

- [ ] Load `EngineConfig` from `ENGINE_DEFAULT_CONFIG` + argv
- [ ] `os::onQuitSignal` (ENG-60: SIGINT / SIGTERM, Windows console handler) → `requestQuit()`
- [ ] Return `Engine::run()`; print the error and return 1 on bad config

## Done when

- Ctrl+C on client and server shuts down cleanly (log shows shutdown, exit code 0)

## Watch out

- Signal handlers may only set an atomic flag

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
