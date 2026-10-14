# ENG-58 · platform_raylib plugin: 2D, then optional 3D

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **15 Part 2** · status and owner live on the board.

## Goal

The raylib backend, then optional 3D (Part 2).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/platform_raylib/`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `plugins/platform_raylib/RaylibPlatform.hpp`

```cpp
class RaylibPlatform final : public IPlatform { /* same methods as SfmlPlatform; pollEvents converts
                                                   raylib polling (IsKeyPressed…) into RawInputEvent */ };
class RaylibRenderer final : public IRenderer   { /* same methods as SfmlRenderer; supports(Render3D) == true */ };
ENGINE_PLUGIN(RaylibPlatform)
```

## Depends on

- [ENG-31](ENG-31.md) — platform_sfml plugin: window, events to InputMap, batching renderer

## Unlocks

- no task waits for this one

## Read first

- [ADR 0007](../adr/0007-sfml-and-raylib-backends.md)

## To implement

- [ ] RaylibPlatform / RaylibRenderer for 2D
- [ ] Polling converted to RawInputEvent
- [ ] `supports(Render3D)` and 3D commands later

## Done when

- The game runs unchanged with `--platform raylib`

## Watch out

- No raylib type outside the plugin

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
