# ENG-47 · Metrics and engine.debug_overlay (ImGui, ImPlot)

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **12 Services** · status and owner live on the board.

## Goal

Metrics and the debug overlay.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/services/Metrics.hpp`
- `src/services/Metrics.cpp`
- `src/systems/DebugOverlaySystem.cpp`

## Depends on

- [ENG-32](ENG-32.md) — Dear ImGui and ImPlot in platform_sfml (debug UI hooks)
- [ENG-26](ENG-26.md) — Scheduler: stages, names, after/before, roles, stats, failure policy

## Unlocks

- no task waits for this one

## Read first

- [ADR 0035](../adr/0035-services-and-tools.md)

## To implement

- [ ] `Metrics`: frame timing, series ring buffers, snapshot
- [ ] `engine.debug_overlay`: console, FPS graph, system timings, lagometer

## Done when

- Overlay shows per-system times matching `systems`

## Watch out

- Recording must stay cheap: no allocation per sample

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
