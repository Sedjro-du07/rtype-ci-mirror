# ENG-25 · Engine components and their registration

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **04 Reflection** · **Wave 4** · status and owner live on the board.

## Goal

The engine's built-in components and their registration.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/components/Position.hpp`
- `include/engine/components/Velocity.hpp`
- `include/engine/components/Sprite.hpp`
- `include/engine/components/Animation.hpp`
- `include/engine/components/Text.hpp`
- `include/engine/components/Collider.hpp`
- `include/engine/components/Lifetime.hpp`
- `include/engine/components/Parallax.hpp`
- `include/engine/components/Script.hpp`
- `src/systems/RegisterEngineTypes.hpp`
- `src/systems/RegisterEngineTypes.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/components/*.hpp`

```cpp
struct Position         { float x = 0, y = 0; };                          ENGINE_COMPONENT(Position)
struct PreviousPosition { float x = 0, y = 0; };                          ENGINE_COMPONENT(PreviousPosition)
struct Velocity         { float dx = 0, dy = 0; };                        ENGINE_COMPONENT(Velocity)
struct Sprite           { TextureHandle texture; Rect frame; int16_t layer = 0; Color tint;
                          Vec2 scale{1, 1}; Vec2 origin; float rotation = 0; bool visible = true; };
                                                                          ENGINE_COMPONENT(Sprite)
struct Animation        { std::vector<Rect> frames; float fps = 10; bool loop = true;
                          float time = 0; uint32_t current = 0; };        ENGINE_COMPONENT(Animation)
struct Text             { std::string value; FontHandle font; uint16_t size = 16; Color color;
                          int16_t layer = 100; bool screenSpace = true; };  ENGINE_COMPONENT(Text)
struct Collider         { Vec2 size; Vec2 offset; uint32_t layer = 1; uint32_t mask = ~0u; bool trigger = false; };
                                                                          ENGINE_COMPONENT(Collider)
struct Lifetime         { float seconds = 0; };                           ENGINE_COMPONENT(Lifetime)
struct Parallax         { float factor = 1; float wrapWidth = 0; };       ENGINE_COMPONENT(Parallax)
// Script.hpp
struct Script           { ScriptHandle handle; std::string file; };       ENGINE_COMPONENT(Script)
```

### `src/systems/RegisterEngineTypes.cpp`

```cpp
void registerEngineTypes(TypeRegistry& types);   // every component above with fields and flags,
                                                  // net components (block 10), engine events (block 6)
```

## Depends on

- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)
- [ENG-30](ENG-30.md) — Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform

## Unlocks

- [ENG-50](ENG-50.md) — Systems: previous_position, movement, hierarchy, parallax, lifetime
- [ENG-51](ENG-51.md) — Collision system: uniform grid, AABB, layers, Collision events
- [ENG-52](ENG-52.md) — Render systems: sprites (interpolated), text, animation, debug colliders

## Read first

- [ADR 0016](../adr/0016-identity-as-components.md)
- [reference 13](../engine/reference/13-components-systems.md)

## To implement

- [ ] Headers of reference 13 with `ENGINE_COMPONENT` (identity and hierarchy are written by ENG-18)
- [ ] `registerEngineTypes`: fields and flags (Position/Velocity/Sprite Replicated, PreviousPosition Transient…) and engine events

## Done when

- Inspector-style dump (`types` console command) lists every engine component with its fields

## Watch out

- Names are reserved: a game type with the same name is a boot error

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
