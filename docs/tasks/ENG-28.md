# ENG-28 · Keys, RawInputEvent, ActionId

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **07 Input** · status and owner live on the board.

## Goal

Device vocabulary and action ids.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/input/Keys.hpp`
- `include/engine/input/RawInputEvent.hpp`
- `include/engine/input/Action.hpp`
- `src/input/Keys.cpp`

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError

## Unlocks

- [ENG-29](ENG-29.md) — InputMap and ActionState: input.yaml, rebinding, accessibility options, masks

## Read first

- [ADR 0030](../adr/0030-actions-and-input-map.md)

## To implement

- [ ] `Key`, `MouseButton`, `GamepadButton`, `GamepadAxis` enums; name ↔ enum tables; `keyName`
- [ ] `RawInputEvent`
- [ ] `ActionId`, `action()` constexpr

## Done when

- Tests: every key name parses back to its enum

## Watch out

- Keys are physical positions (scancodes)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
