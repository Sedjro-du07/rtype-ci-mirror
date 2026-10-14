# ENG-51 · Collision system: uniform grid, AABB, layers, Collision events

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **13 Engine systems** · status and owner live on the board.

## Goal

Collision detection on a uniform grid.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/systems/CollisionSystem.cpp`

## Depends on

- [ENG-25](ENG-25.md) — Engine components and their registration
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events

## Unlocks

- no task waits for this one

## Read first

- [reference 13](../engine/reference/13-components-systems.md)

## To implement

- [ ] Grid of `phys_grid_cell`
- [ ] AABB test
- [ ] Layer/mask both ways
- [ ] Publish `Collision{a,b}` every overlapping tick

## Done when

- Tests: mask rule both ways; 1000 colliders within budget

## Watch out

- Detection only in Part 1: no response

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
