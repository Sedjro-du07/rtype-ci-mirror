# Engine architecture reference

Status: **Locked** (2026-10-09). Every class, interface, function and method of the engine, with
what it does. One page per block. Decisions behind each block are in [`../../adr/README.md`](../../adr/README.md).
How to *use* the engine is in the guide: [`../guide/README.md`](../guide/README.md).
Flat lists: [`../to-implement.md`](../to-implement.md) (what to code) and [`../game-api.md`](../game-api.md) (what games use).

| # | Block | Page | Namespace / folder |
|---|---|---|---|
| 1 | Engine, Context, lifecycle, core types | [01-core.md](01-core.md) | `engine` · `core/` |
| 2 | Modules and plugins | [02-modules.md](02-modules.md) | `engine` · `module/` |
| 3 | ECS | [03-ecs.md](03-ecs.md) | `engine` · `ecs/` |
| 4 | Reflection and serialization | [04-reflection.md](04-reflection.md) | `engine` · `reflection/` |
| 5 | Scheduler | [05-scheduler.md](05-scheduler.md) | `engine` · `scheduler/` |
| 6 | Events | [06-events.md](06-events.md) | `engine` · `events/` |
| 7 | Input | [07-input.md](07-input.md) | `engine` · `input/` |
| 8 | Platform and rendering | [08-rendering.md](08-rendering.md) | `engine` · `render/` |
| 9 | Audio | [09-audio.md](09-audio.md) | `engine` · `audio/` |
| 10 | Network (engine side) | [10-network.md](10-network.md) | `engine` · `net/` |
| 11 | Scenes, prefabs, assets | [11-scenes-assets.md](11-scenes-assets.md) | `engine` · `scene/`, `assets/` |
| 12 | Services, tools, scripting | [12-services.md](12-services.md) | `engine` · `services/`, `script/` |
| — | Built-in components and systems | [13-components-systems.md](13-components-systems.md) | `engine` · `components/`, `systems/` |

## Conventions used in every page

- All engine types live in namespace `engine`. Headers are under `engine/include/engine/<block>/`,
  sources under `engine/src/<block>/` (see [`../folder-tree.md`](../folder-tree.md)).
- Interfaces start with `I`. Concrete classes have no prefix.
- C++23 with the allow-list of ADR 0017. Expected failures return `Result<T>` (`std::expected<T, Error>`).
- "Engine only" means the method is called by the engine itself, never by game code.
- "Loop thread" means the thread running `Engine::loop()`. Unless stated, every method is loop-thread only.
- No type from SFML, raylib, Asio, miniaudio or Lua appears in any engine header.

## Dependency direction between blocks

Headers are layered. A layer includes only layers above it in this list:

```
1. core/         math, Error/Result, Time/Clock, Signal/Subscription, fnv1a64      (no engine deps)
2. ecs/ leaf     Entity, ComponentId, IStorage, SparseSet<T>
3. reflection/   TypeRegistry, TypeInfo, ByteStream, YAML wrappers
4. ecs/          Registry, View, Commands
5. events/ scheduler/ input/
6. render/ audio/ net/ assets/ scene/ services/ script/ module/   (interfaces and services)
7. Engine.hpp, Context.hpp, EngineConfig.hpp   (top: own or expose everything)
```

`Engine.hpp` is included only by the launcher. Game code includes `Context.hpp` and the headers of
the blocks it uses, never `Engine.hpp`.
