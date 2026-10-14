# ENG-08 · Engine: lifecycle, member order, fixed-step loop, error policy

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **01 Core** · status and owner live on the board.

## Goal

The composition root: owns every subsystem, runs boot → load → loop → shutdown.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/Engine.hpp`
- `src/core/Engine.cpp`

## Depends on

- [ENG-07](ENG-07.md) — EngineConfig: YAML file and command-line overrides
- [ENG-14](ENG-14.md) — ModuleManager: load by kind and name, version check, reverse unload
- [ENG-26](ENG-26.md) — Scheduler: stages, names, after/before, roles, stats, failure policy
- [ENG-43](ENG-43.md) — Scene and SceneManager: switch, persistent entities
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events
- [ENG-29](ENG-29.md) — InputMap and ActionState: input.yaml, rebinding, accessibility options, masks
- [ENG-30](ENG-30.md) — Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform
- [ENG-38](ENG-38.md) — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts
- [ENG-46](ENG-46.md) — Console and built-in commands

## Unlocks

- [ENG-09](ENG-09.md) — Context facade
- [ENG-10](ENG-10.md) — Generic launcher main() and signal handling
- [ENG-57](ENG-57.md) — Headless smoke test in CI: client with Null plugins runs 1000 ticks

## Read first

- [ADR 0003](../adr/0003-fixed-timestep-loop.md)
- [ADR 0021](../adr/0021-engine-composition-root.md)
- [ADR 0024](../adr/0024-error-handling-policy.md)
- [reference 01](../engine/reference/01-core.md)

## To implement

- [ ] Members in the order of reference 01 (declaration order = shutdown order)
- [ ] `boot()`: logger, CVars + `loadArchive`, plugins of the role, game module, engine types then `registerTypes`
- [ ] `load()`: engine systems then `registerSystems(role)`, `Scheduler::build`, first scene (or `--scene`), `onStart`
- [ ] `loop()`: fixed step with accumulator clamp, input, `network.poll(ctx)`, FixedUpdate × N, LateUpdate, scene switch, Render, submit, audio, metrics; `--ticks` stop
- [ ] `shutdown()`: `onShutdown`, `saveArchive`, scenes cleared, then RAII
- [ ] Exit codes 1–5 from `BootError` and the client failure policy

## Done when

- Headless run with null plugins and `--ticks 100` exits 0 (ENG-57)
- Each boot failure returns its exit code with a readable message

## Watch out

- Game objects (systems, components, subscriptions) must be destroyed before the game module is unloaded
- Never call game code from the network thread

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
