# Engine architecture

Status: **Locked** (2026-10-09). All twelve blocks are designed: reference in [`reference/`](reference/README.md),
usage guide in [`guide/`](guide/README.md), folder tree in [`folder-tree.md`](folder-tree.md), tasks in [`tasks.md`](tasks.md).
Decisions are recorded as ADRs in [`../adr/README.md`](../adr/README.md). Every class and
interface named here is listed with its status in [`classes.md`](classes.md).

Visual companions (HTML). The three pages share a navigation bar to move from one to another.
Sharing is set per page from its Share menu; check it before sending a link.

| Page | Online | Source |
|---|---|---|
| Architecture snapshot: layers, loop, decisions | [open](https://claude.ai/artifact/8SCfRPzicju1fSpgxRmfWF) | `docs/architecture/engine-architecture.html` |
| Class reference: signatures and diagram | [open](https://claude.ai/artifact/QJCdiQcPYn15WT9iu8eF7B) | `docs/architecture/engine-classes.html` |
| Inventory: everything, with status | [open](https://claude.ai/artifact/FGAmyXhSoPrJycMsLXsoUA) | `docs/architecture/engine-inventory.html` |

## What the engine is

A game-agnostic engine library. R-Type is its first client; a second game (Pong or more) proves
it is generic. The engine owns `main()` and the game loop. Everything replaceable lives in a
`.so` / `.dll` behind an interface the core defines, and that includes the game itself.

Proof of decoupling: `engine/` builds alone, with no `#include` of anything R-Type.

## Four layers

Dependencies only go down. A layer knows its own interfaces and the ones below it, never the ones
above. CMake enforces it: a forbidden include breaks the build.

```
┌──────────────────────────────────────────────────────────────────┐
│ 4. CONTENT        YAML scenes and prefabs, scripts, sprites, sounds │  game designers
├──────────────────────────────────────────────────────────────────┤
│ 3. GAME MODULE    R-Type rules: Bydos, the Force, waves, score      │  gameplay developers
├──────────────────────────────────────────────────────────────────┤
│ 2. ENGINE CORE    loop · ECS · TypeRegistry · InputMap · EventBus · │  engine developers
│                   scenes · assets · plugin loader · services        │
├──────────────────────────────────────────────────────────────────┤
│ 1. PLUGINS        platform (SFML / raylib) · audio · network ·      │  engine developers
│                   scripting                                         │
├──────────────────────────────────────────────────────────────────┤
│ 0. OS + LIBRARIES Linux · Windows · SFML · raylib · Asio · miniaudio│  third parties
└──────────────────────────────────────────────────────────────────┘
```

## Who loads what

```
r-type_client / r-type_server        generic launcher shipped by the engine (ADR 0005)
            │ boots
            ▼
┌────────────────────────── ENGINE CORE (libengine) ──────────────────────────┐
│ Engine · Context · loop + Clock · ECS · TypeRegistry · InputMap · EventBus  │
│ Scheduler · SceneManager · AssetManager · RenderQueue · ModuleManager · Log │
└──────────────┬──────────────────────────────────────────────────────────────┘
               │ talks only through interfaces
   IPlatform · IRenderer · IAudio · INetwork · IScripting · IGameModule
               │ loads at runtime (.so / .dll), or statically (ADR 0009)
   ┌───────────┼───────────┬──────────────┬─────────────┬──────────────┐
platform_sfml  platform_raylib  audio_miniaudio  network_asio  scripting   rtype_game
(Part 1)       (Part 2, 3D opt.)                                (Track 3)   (2nd game = another module)
```

| Module | r-type_client | r-type_server (room) | editor (Part 2) |
|---|---|---|---|
| engine core | yes | yes | yes |
| platform_sfml / platform_raylib | one of them | never | one of them |
| audio_miniaudio | yes | no | preview |
| network_asio | yes | yes | no |
| scripting | yes | yes | yes |
| rtype_game | client systems | server systems | types only |

## Lifecycle

`Engine::run()` fixes the order (Template Method); the game fills hooks.

```
run()
 ├─ boot()      logger → config → plugins of the role (API version check)
 │              → game module (create_game) → game.registerTypes(types)
 ├─ load()      game.registerSystems(role, scheduler) → load game.firstScene()
 │              → request assets → game.onStart(ctx)
 ├─ loop()      while running: poll → FixedUpdate ×N → LateUpdate → Render(α) → submit
 └─ shutdown()  game.onShutdown(ctx) → clear scenes → members destroyed in reverse order
```

Shutdown order comes from the declaration order of `Engine`'s members: the module manager is
declared early so it is destroyed late, after the scenes and the scheduler that still hold code
from the game module, and the logger outlives everyone who writes to it.

## The game loop

The loop lives in `Engine::loop()`, written once, for every game and both roles (ADR 0003).

```cpp
const double step = 1.0 / config_.tickRate;       // 1/60 s
double accumulator = 0.0;
while (running_) {
    accumulator += clock_.restart();
    platform().pollEvents(input_);                 // 1 · poll: devices → actions
    network().poll(events_);
    while (accumulator >= step) {                  // 2 · fixed update, same dt everywhere
        scheduler_.run(Stage::FixedUpdate, ctx);
        commands_.apply();                         //     structural changes (ADR 0015)
        accumulator -= step;
    }
    scheduler_.run(Stage::LateUpdate, ctx);        // 3 · late update
    events_.dispatch();
    time_.alpha = accumulator / step;              // 4 · render with interpolation
    scheduler_.run(Stage::Render, ctx);
    platform().renderer().submit(renderQueue_);    // 5 · one call into the plugin (ADR 0006)
    renderQueue_.clear();
}
```

The server runs the same loop. Its platform and renderer are Null Objects and no system is
registered in `Stage::Render`.

## Error handling

| Kind | Example | Response |
|---|---|---|
| Boot error | missing plugin, API version mismatch, bad scene | clear message, `run()` returns a non-zero exit code per stage (1 config, 2 plugin, 3 game module, 4 scene) |
| Exception in a system | bug in `bydosAiSystem` | server: caught by the Scheduler, logged, system disabled, game continues; client: logged, clean exit |
| Signal | `SIGINT`, `SIGTERM` | `requestQuit()`: the frame finishes, shutdown runs, logs are flushed |

No exception may leave an `extern "C"` function. Exceptions across C++ interfaces between modules
built with the same toolchain are allowed.

## Server and engine boundary

The server is the engine in its server role, plus the game's server-side rules (ADR 0011).
Test for each feature: *would an online Pong need it?* Yes → engine. R-Type only → game module.
Deployment → application.

## Language, data and tooling decisions

- **C++23 with an allow-list** (ADR 0017): all of C++20 except modules, plus `std::expected`,
  `std::move_only_function`, `std::print`, deducing this, `std::to_underlying`, `std::unreachable`.
  Fallback `tl::expected`.
- **Data in YAML, behaviour in scripts** (ADR 0018): config, scenes and prefabs are YAML; waves,
  boss phases and special AI are scripts referenced from prefabs.
- **Lua first** (ADR 0019): `scripting_lua` (Lua + sol2) is the first `IScripting` plugin; a custom
  language stays optional, as a second plugin or a small pattern language.
- **Game module helpers** (ADR 0020): `ENGINE_GAME_MODULE(Class)` and `ENGINE_PLUGIN(Class)` generate
  the entry points for both linking modes; `engine_add_game_module` builds a game module.

## Open items

None for the engine. The protocol bytes, the server (lobby, rooms) and the game belong to the next parts.
