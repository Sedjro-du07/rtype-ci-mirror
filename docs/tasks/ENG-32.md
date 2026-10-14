# ENG-32 · Dear ImGui and ImPlot in platform_sfml (debug UI hooks)

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **08 Rendering** · status and owner live on the board.

## Goal

Dear ImGui and ImPlot inside the SFML plugin for the console and metrics.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/platform_sfml/SfmlImGui.cpp`

## Depends on

- [ENG-31](ENG-31.md) — platform_sfml plugin: window, events to InputMap, batching renderer

## Unlocks

- [ENG-47](ENG-47.md) — Metrics and engine.debug_overlay (ImGui, ImPlot)
- [ENG-59](ENG-59.md) — Editor (Track 1): hierarchy, inspector, scene save

## Read first

- [ADR 0031](../adr/0031-render-api.md)
- [ADR 0035](../adr/0035-services-and-tools.md)

## To implement

- [ ] ImGui-SFML init / shutdown
- [ ] `beginDebugUi` / `endDebugUi`

## Done when

- The console opens with F1 and the FPS graph draws

## Watch out

- ImGui must not eat game input when the console is closed

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
