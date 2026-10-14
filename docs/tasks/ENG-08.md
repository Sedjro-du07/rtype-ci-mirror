# ENG-08 · Engine: lifecycle, member order, fixed-step loop, error policy

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **01 Core** · **Wave 10** · status and owner live on the board.

## Goal

The composition root: owns every subsystem, runs boot → load → loop → shutdown.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/Engine.hpp`
- `src/core/Engine.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/Engine.hpp`

```cpp
class Engine {
public:
    explicit Engine(EngineConfig config);   // stores the config only; no work happens here
    ~Engine();                              // members destroyed in reverse declaration order

    int  run();                             // boot → load → loop → shutdown; returns the exit code
    void requestQuit() noexcept;            // thread-safe; the current frame finishes, then shutdown runs

private:
    void boot();       // logger, CVars, plugins of the role (version check), game module, registerTypes
    void load();       // engine systems + registerSystems(role), Scheduler::build, first scene, onStart
    void loop();       // fixed-step loop (see below) while running_
    void shutdown();   // onShutdown, scenes cleared, scripts unloaded; RAII destroys the rest

    // Declaration order = construction order; destruction is the reverse (ADR 0021).
    EngineConfig     config_;
    Logger           logger_;        // outlives everything that logs
    CVars            cvars_;
    ModuleManager    modules_;       // owns the .so/.dll: destroyed after everything using their code
    TypeRegistry     types_;
    EventBus         events_;
    Metrics          metrics_;
    Console          console_;
    AssetManager     assets_;
    InputMap         input_;
    NetworkSession   network_;
    Scheduler        scheduler_;     // holds game systems: destroyed before modules_
    SceneManager     scenes_;        // holds game components: destroyed before modules_
    Commands         commands_;
    RenderQueue      renderQueue_;
    Time             time_;
    Clock            clock_;
    std::atomic_bool running_{false};
};
```

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
