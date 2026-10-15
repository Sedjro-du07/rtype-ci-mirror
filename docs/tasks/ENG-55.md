# ENG-55 · Unit tests: scheduler, events, input, scenes, session

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **14 Tests & bench** · **Wave 10** · status and owner live on the board.

## Goal

Tests for scheduler, events, input, scenes and the session core.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `tests/scheduler/`
- `tests/events/`
- `tests/input/`
- `tests/scene/`
- `tests/net/`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `tests/`

```text
tests/scheduler/ OrderingTest, CycleTest, FailurePolicyTest (server disables, critical ends, client code 5)
tests/events/    DispatchOrderTest, PublishDuringDispatchTest, ThreadInboxTest
tests/input/     EdgeLatchTest, HoldToToggleTest, AutoRepeatTest, DeadzoneTest, MaskTest
tests/scene/     TwoPassLoadTest, UnknownComponentTest, SwitchPersistentTest, SaveRoundTripTest
tests/net/       MalformedDatagramTest, OversizedDatagramTest, TimeoutTest (session core only)
```

## Depends on

- [ENG-26](ENG-26.md) — Scheduler: stages, names, after/before, roles, stats, failure policy
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events
- [ENG-29](ENG-29.md) — InputMap and ActionState: input.yaml, rebinding, accessibility options, masks
- [ENG-43](ENG-43.md) — Scene and SceneManager: switch, persistent entities
- [ENG-38](ENG-38.md) — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts

## Unlocks

- no task waits for this one

## Read first

- [reference 05](../engine/reference/05-scheduler.md)

## To implement

- [ ] One folder per block
- [ ] Session: malformed datagrams, timeouts

## Done when

- All green in CI

## Watch out

- Network tests on loopback only, with fixed ports avoided (port 0)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
