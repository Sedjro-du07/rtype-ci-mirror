# ENG-26 · Scheduler: stages, names, after/before, roles, stats, failure policy

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **05 Scheduler** · status and owner live on the board.

## Goal

Named systems in fixed stages, ordered by constraints, timed, with the failure policy (ADR 0024, 0028).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/scheduler/Stage.hpp`
- `include/engine/scheduler/System.hpp`
- `include/engine/scheduler/Scheduler.hpp`
- `src/scheduler/Scheduler.cpp`

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError
- [ENG-44](ENG-44.md) — Logger and sinks (console, file, developer console)

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-47](ENG-47.md) — Metrics and engine.debug_overlay (ImGui, ImPlot)
- [ENG-50](ENG-50.md) — Systems: previous_position, movement, hierarchy, parallax, lifetime
- [ENG-55](ENG-55.md) — Unit tests: scheduler, events, input, scenes, session

## Read first

- [ADR 0024](../adr/0024-error-handling-policy.md)
- [ADR 0028](../adr/0028-named-ordered-systems.md)
- [reference 05](../engine/reference/05-scheduler.md)

## To implement

- [ ] `add(stage, name, fn)` + `SystemBuilder` (after, before, roles, disabled, critical)
- [ ] `build`: topological sort per stage; unknown name or cycle → error
- [ ] `run`: time each system, catch exceptions, apply the policy per role
- [ ] `setEnabled`, `stats`, `clearGameSystems`

## Done when

- Tests: ordering, cycle message, server disables a throwing system, critical ends the session, client quits with code 5

## Watch out

- Ties keep registration order: deterministic

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
