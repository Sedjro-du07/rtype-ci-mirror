# ENG-09 · Context facade

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **01 Core** · status and owner live on the board.

## Goal

The only door from game code into the engine (ADR 0022).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/Context.hpp`
- `src/core/Context.cpp`

## Depends on

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy

## Unlocks

- no task waits for this one

## Read first

- [ADR 0022](../adr/0022-context-facade.md)
- [reference 01](../engine/reference/01-core.md)

## To implement

- [ ] Accessors of reference 01 forwarding to `Engine` members
- [ ] Private constructor, `friend class Engine`

## Done when

- A game test module compiles using only `Context.hpp` and block headers, never `Engine.hpp`

## Watch out

- Context holds no state of its own; never cache it in game code

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
