# ENG-27 · EventBus: queued, immediate, thread inbox; engine events

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **06 Events** · status and owner live on the board.

## Goal

Typed queued events with RAII subscriptions, plus the engine's own events.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/events/EventBus.hpp`
- `include/engine/events/EngineEvents.hpp`
- `src/events/EventBus.cpp`

## Depends on

- [ENG-06](ENG-06.md) — Signal and RAII Subscription
- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-34](ENG-34.md) — IAudio, PlayParams, buses, captions
- [ENG-38](ENG-38.md) — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts
- [ENG-43](ENG-43.md) — Scene and SceneManager: switch, persistent entities
- [ENG-46](ENG-46.md) — Console and built-in commands
- [ENG-48](ENG-48.md) — IScripting, Script component, script systems
- [ENG-51](ENG-51.md) — Collision system: uniform grid, AABB, layers, Collision events
- [ENG-55](ENG-55.md) — Unit tests: scheduler, events, input, scenes, session

## Read first

- [ADR 0029](../adr/0029-queued-event-bus.md)
- [reference 06](../engine/reference/06-events.md)

## To implement

- [ ] `subscribe`, `publish` (queued), `emit` (immediate), `publishFromAnyThread` (mutex inbox)
- [ ] `dispatch`: deliver in publish order; events published during dispatch wait
- [ ] Engine events of reference 06 (player events included)

## Done when

- Tests: order, publish during dispatch, unsubscribe during dispatch, thread inbox

## Watch out

- Events never leave the process

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
