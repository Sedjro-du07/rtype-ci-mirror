# R-Type engine — Design Pattern Decisions

Language: **C++**. This is a decision log. Each entry records a pattern, the problem it answers in
this engine, where it lives, and its verdict. Entries are appended as decisions are made.

Reference: Robert Nystrom, *Game Programming Patterns* (gameprogrammingpatterns.com).

## Context: what this program really is

Stripped of R-Type, the engine is four things:

1. A **loop** that advances a simulation at a fixed rate and draws at any rate.
2. A **data store** (ECS) that systems read and write every tick.
3. A **plugin host** that loads backends and the game itself at runtime.
4. A **content pipeline** that turns text files into entities.

A pattern earns its place by solving a problem in one of those four, not by being on a list.

---

## Architecture

| Pattern | Verdict | Where | Problem |
|---|---|---|---|
| Entity-Component-System | **Adopted** | `Registry`, components, systems | inheritance trees explode (the Force: moves, collides, shoots, attaches) |
| Layers | **Adopted** | content / game / engine / plugins | everything depends on everything; CMake forbids upward includes |
| Microkernel (plugin architecture) | **Adopted** | core + plugins + game module | interchangeable backends, game as a module |
| Composition root | **Adopted** | `Engine` | one place creates and wires every subsystem |

## Loop

| Pattern | Verdict | Where | Problem |
|---|---|---|---|
| Game Loop | **Adopted** | `Engine::loop()` | game time must not depend on CPU speed |
| Update Method, as systems | **Adopted** | `Scheduler` + systems | advance every entity each tick, in batches instead of per-object `update()` |
| Double Buffer | **Adopted** | previous/current state, `RenderQueue` | draw between two simulation states |
| Template Method | **Adopted** | `Engine::run()` + `IGameModule` hooks | the game fills steps without changing their order |

## Creation

| Pattern | Verdict | Where | Problem |
|---|---|---|---|
| Abstract Factory | **Adopted** | `create_plugin()`, `create_game()` | create a backend without knowing which |
| Prototype | **Adopted** | prefabs | spawn 50 identical Bydos, override only differences |
| Type Object | **Adopted** | enemy kinds as YAML data | 30 enemy kinds must not be 30 classes |
| Object Pool | **Adopted** | entity slots + generation | thousands of missiles created and destroyed |

## Communication

| Pattern | Verdict | Where | Problem |
|---|---|---|---|
| Mediator (event bus) | **Adopted** | `EventBus` | damage triggers sound, FX, network, score without knowing them |
| Observer | **Adopted** | `Registry` hooks | react when a component is created, replaced, destroyed |
| Command | **Adopted** | `Commands`, `DrawCmd`, input actions | separate deciding from executing |
| Event Queue | **Adopted** | commands applied at end of stage, queued events | events fired mid-system would see a half-updated world |

## Adaptation

| Pattern | Verdict | Where | Problem |
|---|---|---|---|
| Bridge / Strategy | **Adopted** | `IRenderer` ↔ SFML, raylib | one abstraction, several implementations |
| Adapter | **Adopted** | each plugin, `DynamicLibrary` | raylib polls, the engine expects events; OS loaders differ |
| Facade | **Adopted** | `Context` | game code should not juggle ten subsystems |

## Services

| Pattern | Verdict | Where | Problem |
|---|---|---|---|
| Dependency Injection | **Adopted** | `Context`, services passed to modules | everyone needs logger, assets, config |
| Singleton | **Rejected** | — | each `.so` gets its own copy; hidden dependencies; untestable |
| Null Object | **Adopted** | `NullAudio`, `NullRenderer`, `NullPlatform` | the server has no audio or rendering; no `nullptr` checks |
| Registry | **Adopted** | `TypeRegistry`, ECS `Registry` | store and find, never decide |
| RAII | **Adopted** | `Engine` member order, `DynamicLibrary`, handles | release in reverse order, even on error |
| Flyweight | **Adopted** | asset handles | 300 Bydos must not hold 300 texture copies |

## Performance and content

| Pattern | Verdict | Where | Problem |
|---|---|---|---|
| Spatial Partition | **Adopted** | `engine.collision` uniform grid | pairwise collision tests are O(n²) |
| Data Locality | **Adopted** | `SparseSet<T>` dense arrays | cache misses when iterating |
| Bytecode / Interpreter | **Adopted** | Lua through `IScripting` | behaviours without recompiling |
| Service Locator | **Rejected** | — | `Context` injects services explicitly |
| Dirty Flag | **Not used** | — | hierarchy is shallow; `engine.hierarchy` recomputes child positions each tick |
| State | Game level | boss phases, menus in the game module or scripts | not an engine concern |

## Anti-patterns we refuse

- A god `GameManager` that does everything.
- Singletons reached through globals.
- Deep inheritance trees for game objects.
- Pointers or references to components kept across frames (storages relocate).

---

## Decision — Null Object: **ADOPTED**

### The problem

The server has no window, no renderer and no audio, but runs the same loop and gives systems the
same `Context`. Returning `nullptr` from `ctx.audio()` forces a check at every call; one missing
check crashes the server mid-game, and the bug only shows on the server.

### The verdict

`Context::audio()` returns `IAudio&`. On the server it is a `NullAudio` whose methods do nothing.
The choice is made once, in `Engine::boot()`. Same for `NullRenderer` and `NullPlatform`.

### The limit

A Null Object hides absence on purpose. It is used only where doing nothing is correct: server,
CI tests, a client without a sound device. A client whose rendering plugin fails to load stops
with a clear message. Every Null Object in use is logged once at startup.

## Decision — Context as Facade with injected services: **ADOPTED**

### The problem

Systems need the world, time, actions, audio, assets. Handing them `Engine&` lets game code break
the engine and couples it to every engine header. Singletons break across `.so` boundaries.

### The verdict

A non-owning `Context` with references to the subsystems, built only by `Engine` (private
constructor, `friend class Engine`), used only on the loop thread. Read-only access for `input()`
and `time()`. `world()` is a method because the active scene changes.

### Alternatives rejected

Passing every dependency to every system (signatures grow with each need); Bevy-style typed
system parameters (heavy metaprogramming for six weeks); a global service locator (duplicated per
module).
