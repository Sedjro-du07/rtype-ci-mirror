# ENG-52 · Render systems: sprites (interpolated), text, animation, debug colliders

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **13 Engine systems** · **Wave 5** · status and owner live on the board.

## Goal

Built-in drawing and animation.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/systems/RenderSystems.cpp`
- `src/systems/AnimationSystem.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/systems/RenderSystems.cpp`

```cpp
void renderSprites(Context& ctx);   // "engine.render.sprites" Render, client/editor: Position lerped with alpha
void renderText(Context& ctx);      // "engine.render.text"    Render, after sprites
void renderDebug(Context& ctx);     // "engine.render.debug"   Render, when r_debug_colliders
```

### `src/systems/AnimationSystem.cpp`

```cpp
void animation(Context& ctx);       // "engine.animation" FixedUpdate, client/editor: advances Animation, writes Sprite.frame
```

## Depends on

- [ENG-25](ENG-25.md) — Engine components and their registration
- [ENG-30](ENG-30.md) — Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform

## Unlocks

- no task waits for this one

## Read first

- [ADR 0003](../adr/0003-fixed-timestep-loop.md)
- [ADR 0031](../adr/0031-render-api.md)

## To implement

- [ ] render.sprites interpolated with alpha
- [ ] render.text
- [ ] render.debug (collider outlines)
- [ ] animation (frames, loop)

## Done when

- Smooth motion at 144 Hz display with 60 Hz simulation

## Watch out

- Render systems only fill the RenderQueue

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
