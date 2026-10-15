# ADR 0021: Engine is the composition root and member order is shutdown order

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 1

## Context

Every subsystem must be created, wired and destroyed in a safe order. Objects created by a plugin must die before the plugin is unloaded; components and systems whose code lives in the game module must die before that module is unloaded; the logger must outlive everyone who writes to it.

## Decision

`Engine` owns every subsystem as a direct member and is the only place that creates and wires them. `run()` fixes the lifecycle (boot, load, loop, shutdown) as a Template Method; `boot()`, `load()`, `loop()` and `shutdown()` are private. Members are declared so that reverse destruction is safe: `Logger` before `ModuleManager`, `ModuleManager` before `Scheduler` and `SceneManager`. `requestQuit()` is `noexcept` and thread-safe through `std::atomic_bool running_`. The engine ships `main()` (ADR 0005). `IGameModule` hooks: `registerTypes`, `registerSystems(Role, Scheduler&)`, `firstScene`, optional `onStart` / `onShutdown`.

## Alternatives considered

- **A hand-written `shutdown()` destroying members explicitly:** breaks the day a member is added and forgotten, or an exception skips it.
- **Subsystems behind `std::unique_ptr`:** possible null states and no gain, since their lifetime is exactly the engine's.

## Consequences

- Reordering two members can create a crash at exit with no compile error: the order is commented in the code and documented here.
- `Engine.hpp` includes every subsystem header; only launchers include it (Pimpl not needed).
