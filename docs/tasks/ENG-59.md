# ENG-59 · Editor (Track 1): hierarchy, inspector, scene save

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **15 Part 2** · **Wave 10** · status and owner live on the board.

## Goal

The editor (Part 2, Track 1).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `tools/editor/`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

> Part 2: the editor classes are designed when Track 1 opens. Expected pieces: an Editor role, a hierarchy panel, an inspector driven by TypeRegistry, scene save through SceneLoader::save.

## Depends on

- [ENG-43](ENG-43.md) — Scene and SceneManager: switch, persistent entities
- [ENG-32](ENG-32.md) — Dear ImGui and ImPlot in platform_sfml (debug UI hooks)

## Unlocks

- no task waits for this one

## Read first

- [ADR 0001](../adr/0001-hybrid-engine-identity.md)

## To implement

- [ ] Hierarchy, inspector driven by reflection, scene save

## Done when

- A level edited and saved reloads identically

## Watch out

- Editor role uses the same engine; no editor code in the core

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
