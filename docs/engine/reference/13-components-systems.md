# Built-in components and systems

Status: **Locked**. ADRs 0016, 0026, 0031, 0033, 0035.
Folder: `engine/include/engine/components/`, `engine/src/systems/`.

Every game receives these without writing them. Engine components are registered before
`IGameModule::registerTypes`; engine systems before `IGameModule::registerSystems`.

## Components

| Component | Fields | Flags | Used by |
|---|---|---|---|
| `Position` | `float x, y` | Replicated | everything that has a place in the world |
| `PreviousPosition` | `float x, y` | Transient, EditorHidden | interpolation (ADR 0003) |
| `Velocity` | `float dx, dy` (units per second) | Replicated | `engine.movement` |
| `Sprite` | `TextureHandle texture; Rect frame; int16_t layer; Color tint; Vec2 scale; Vec2 origin; float rotation; bool visible` | Replicated | `engine.render.sprites` |
| `Animation` | `std::vector<Rect> frames; float fps; bool loop; float time; uint32_t current` | | `engine.animation` |
| `Text` | `std::string value; FontHandle font; uint16_t size; Color color; int16_t layer; bool screenSpace` | | `engine.render.text` |
| `Collider` | `Vec2 size; Vec2 offset; uint32_t layer; uint32_t mask; bool trigger` | | `engine.collision` |
| `Lifetime` | `float seconds` | | `engine.lifetime` (missiles, particles) |
| `Parallax` | `float factor; float wrapWidth` | | `engine.parallax` (starfields) |
| `Name` | `std::string value` | | `Registry::find`, console, editor |
| `Uuid` | `uint64_t hi, lo` | EditorHidden | scene files, `Registry::find(Uuid)` |
| `Tags` | `small_vector<uint64_t> ids` | | text tags from YAML and scripts |
| `Parent` | `Entity value; Vec2 offset` | | `engine.hierarchy` |
| `Children` | `std::vector<Entity> list` | Transient | `Registry::children` |
| `Script` | see block 12 | | `engine.script.*` |
| `NetworkId`, `NetPrefab`, `NetOwner`, `RemoteInput`, `NetInterpolation`, `NetPriority` | see block 10 | | `engine.net.*` |

All of them, the engine events (`QuitRequested`, `WindowResized`, `WindowFocusChanged`, `AppSuspended`,
`AppResumed`, `Collision`, `SceneLoaded`, `SceneUnloading`, `Caption`, `ConsoleCommand`) and `PreviousPosition`
(in `Position.hpp`) are registered by `registerEngineTypes(TypeRegistry&)` (`src/include/systems/RegisterEngineTypes.hpp`).
A game type with an engine name is a boot error (`DuplicateName`, exit code 3).

YAML forms that are not field by field:

| Component | In a file |
|---|---|
| `Uuid` | `Uuid: a1f3c2d4e5f60718` (hex) |
| `Tags` | `Tags: [player, flying]`; written back as ids (`["#5a1f…"]`) because only the hash is kept |
| `Script` | `Script: { file: scripts/boss.lua }` or `Script: scripts/boss.lua`; the handle is loaded, not saved |

## Systems

| Name | Stage | Roles | After | Does |
|---|---|---|---|---|
| `engine.net.receive_input` | FixedUpdate | server | — | client actions → `RemoteInput` |
| `engine.net.apply_snapshot` | FixedUpdate | client | — | snapshots → components |
| `engine.previous_position` | FixedUpdate | all | net systems | copies `Position` into `PreviousPosition` |
| `engine.movement` | FixedUpdate | all | `engine.previous_position` | `Position += Velocity * fixedDelta` |
| `engine.hierarchy` | FixedUpdate | all | `engine.movement` | child `Position` = parent `Position` + `Parent.offset` |
| `engine.parallax` | FixedUpdate | client | `engine.movement` | wraps parallax layers horizontally |
| `engine.lifetime` | FixedUpdate | all | — | decrements, destroys at 0 through `Commands` |
| `engine.animation` | FixedUpdate | client, editor | — | advances `Animation`, updates `Sprite.frame` |
| `engine.collision` | FixedUpdate | all | `engine.hierarchy` | uniform-grid broad phase + AABB, layer/mask filter, publishes `Collision` |
| `engine.script.*` | FixedUpdate | all | `engine.collision` | block 12 |
| `engine.net.assign_ids` | LateUpdate | server | — | block 10 |
| `engine.net.snapshot` | LateUpdate | server | `engine.net.assign_ids` | block 10 |
| `engine.net.send_input` | LateUpdate | client | — | block 10 |
| `engine.net.interpolate` | Render | client | — | block 10 |
| `engine.render.sprites` | Render | client, editor | `engine.net.interpolate` | `Position` lerped with `alpha` → `RenderQueue::sprite` |
| `engine.render.text` | Render | client, editor | `engine.render.sprites` | `Text` → `RenderQueue::text` |
| `engine.render.debug` | Render | client, editor | — | collider outlines when `r_debug_colliders` |
| `engine.debug_overlay` | Render | client, editor | last | console and metrics (block 12) |

### How the systems are written — `src/include/systems/EngineSystems.hpp` (private)

Each system is a function taking exactly what it uses, so it can be tested without an engine:

```cpp
void previousPosition(Registry&);                 void movement(Registry&, double dt);
void hierarchy(Registry&);                        void parallax(Registry&);
void lifetime(Registry&, Commands&, double dt);   void animation(Registry&, double dt);
void collision(Registry&, EventBus&, float cellSize);
void renderSprites(Registry&, RenderQueue&, double alpha);
void renderText(Registry&, RenderQueue&);          void renderDebug(Registry&, RenderQueue&);

struct SystemTargets { Registry& world; Commands& commands; EventBus& events; const Time& time;
                       RenderQueue* render; float gridCell; bool debugColliders; };
using TargetsOf = std::function<SystemTargets(Context&)>;
void registerEngineSystems(Scheduler&, TargetsOf);   // names, stages, roles and order of the table above
```

`registerEngineSystems` wraps each function in a `SystemFn` that asks `targetsOf(ctx)` for its world,
commands, time…: the engine (ENG-08/09) passes the function that reads them from `Context`, and
`phys_grid_cell` / `r_debug_colliders` until the CVars exist (ENG-45). The `after` constraints on the net
systems and `engine.debug_overlay` are added with those systems (ENG-39, ENG-47).

Conventions: a collider box is `Position + offset` (top-left) with `size`; boxes that only touch do not
collide; `Collision{a, b}` has `a` before `b` by entity index, one per pair per tick, in a stable order.
`engine.parallax` keeps `x` in `(-wrapWidth, 0]`; `factor` shifts the drawn sprite by the camera's travel
from its default centre times `1 - factor`. `engine.render.sprites` draws `PreviousPosition` lerped to
`Position` with `alpha`; `engine.render.debug` draws outlines on layer 30000 (yellow for triggers).
`engine.lifetime` records one `destroy` when the time runs out.

Collision rule: a pair (a, b) collides when their boxes overlap and `(a.layer & b.mask) != 0` and
`(b.layer & a.mask) != 0`. `engine.collision` only detects and publishes `Collision` every tick the
pair overlaps; there is no physical response in Part 1. `trigger` marks sensors for game rules and
for the future physics response. The grid cell size is the CVar `phys_grid_cell` (default 64).

Game systems can be placed with `after("engine.movement")`, `before("engine.collision")` and so on.
