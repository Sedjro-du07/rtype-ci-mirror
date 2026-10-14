# ENG-57 · Headless smoke test in CI: client with Null plugins runs 1000 ticks

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **14 Tests & bench** · status and owner live on the board.

## Goal

A CI smoke test: a client with null plugins runs 1000 ticks.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `tests/headless/`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `tests/headless/`

```text
tests/headless/CMakeLists.txt   add_test(NAME headless_smoke COMMAND r-type_test_client --platform null --audio null --ticks 1000)
tests/headless/config.yaml      role: client, game: testgame, scripting: ""
```

## Depends on

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-15](ENG-15.md) — Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting

## Unlocks

- no task waits for this one

## Read first

- [ADR 0023](../adr/0023-null-object-services.md)

## To implement

- [ ] Test game + `--platform null --audio null --ticks 1000`

## Done when

- Exit code 0 in CI on every push

## Watch out

- No port opened unless the test game calls startServer

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
