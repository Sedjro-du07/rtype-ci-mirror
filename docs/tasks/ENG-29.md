# ENG-29 · InputMap and ActionState: input.yaml, rebinding, accessibility options, masks

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **07 Input** · status and owner live on the board.

## Goal

Turn raw input into actions with rebinding and accessibility options.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/input/InputMap.hpp`
- `include/engine/input/ActionState.hpp`
- `src/input/InputMap.cpp`
- `src/input/ActionState.cpp`

## Depends on

- [ENG-28](ENG-28.md) — Keys, RawInputEvent, ActionId
- [ENG-23](ENG-23.md) — YAML wrappers and field codecs

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-31](ENG-31.md) — platform_sfml plugin: window, events to InputMap, batching renderer
- [ENG-39](ENG-39.md) — Replication: NetworkIdMap, net components, snapshot and input systems
- [ENG-55](ENG-55.md) — Unit tests: scheduler, events, input, scenes, session

## Read first

- [ADR 0030](../adr/0030-actions-and-input-map.md)
- [reference 07](../engine/reference/07-input.md)

## To implement

- [ ] `InputMap`: load / save input.yaml, bind, unbind, boundTo, captureNextBinding, options
- [ ] Edge latching in `beginTick`, hold-to-toggle, auto-repeat, deadzone, one-handed set
- [ ] `ActionState`: pressed, held, released, axis, toMask / fromMask

## Done when

- Tests: a press and release between two ticks is seen once; options behave as documented

## Watch out

- Games read actions, never keys

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
