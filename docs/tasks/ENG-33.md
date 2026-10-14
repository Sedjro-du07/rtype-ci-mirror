# ENG-33 · Colour-blindness filters and UI scale in platform_sfml

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **08 Rendering** · status and owner live on the board.

## Goal

Colour-blindness filters and UI scale (accessibility).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/platform_sfml/ColorFilter.frag`

## Depends on

- [ENG-31](ENG-31.md) — platform_sfml plugin: window, events to InputMap, batching renderer

## Unlocks

- no task waits for this one

## Read first

- [ADR 0031](../adr/0031-render-api.md)

## To implement

- [ ] Fragment shader with protanopia, deuteranopia, tritanopia matrices and high contrast
- [ ] `setColorFilter`, UI scale applied to screen-space draws

## Done when

- `r_color_filter` changes the picture live; `r_ui_scale 1.5` enlarges HUD text

## Watch out

- Without shader support, `supports(ColorFilter)` returns false

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
