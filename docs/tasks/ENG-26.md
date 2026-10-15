# ENG-26 · Scheduler: stages, names, after/before, roles, stats, failure policy

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **05 Scheduler** · **Wave 3** · status and owner live on the board.

## Goal

Named systems in fixed stages, ordered by constraints, timed, with the failure policy (ADR 0024, 0028).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/scheduler/Stage.hpp`
- `include/engine/scheduler/System.hpp`
- `include/engine/scheduler/Scheduler.hpp`
- `src/scheduler/Scheduler.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/scheduler/Stage.hpp`

```cpp
enum class Stage : uint8_t {
    FixedUpdate,  // N times per frame at 1/tickRate: input, movement, AI, collisions, damage
    LateUpdate,   // once per frame after simulation: network send, hierarchy, cleanup
    Render        // once per frame, client and editor only: fill the RenderQueue
};
```

### `include/engine/scheduler/System.hpp`

```cpp
using SystemFn = std::move_only_function<void(Context&)>;
enum RoleMask : uint8_t { ClientRole = 1, ServerRole = 2, EditorRole = 4, AllRoles = 7 };
```

### `include/engine/scheduler/Scheduler.hpp`

```cpp
class SystemBuilder {
public:
    SystemBuilder& after(std::string_view other);   // run after this system (same stage)
    SystemBuilder& before(std::string_view other);  // run before this system (same stage)
    SystemBuilder& roles(uint8_t mask);             // only registered for these roles
    SystemBuilder& disabled();                      // starts disabled (enable from console or code)
    SystemBuilder& critical();                      // if it faults on the server, end the session cleanly
};

struct SystemStats {
    std::string_view name;
    Stage            stage;
    double           lastMs, averageMs;  // timing for the profiler overlay
    bool             enabled;
    bool             faulted;            // threw and was disabled
    std::string      lastError;
};

class Scheduler {
public:
    explicit Scheduler(Role role, Logger& log);

    // Names are unique, "<module>.<system>": "engine.movement", "rtype.bydos_ai".
    template <class F> SystemBuilder add(Stage stage, std::string name, F fn);   // F copyable: one copy per room
    using SystemFactory = std::function<SystemFn()>;
    SystemBuilder addFactory(Stage stage, std::string name, SystemFactory make); // builds one instance per room
    std::vector<SystemFn> instantiate(Stage stage) const;   // engine only: the systems of one room, in order

    Result<void> build();                 // engine only: topological sort per stage;
                                          // unknown name in after/before or a cycle → ScheduleCycle
    void run(Stage stage, Context& ctx);  // engine only: calls each enabled system in order, times it,
                                          // catches exceptions (policy below)
    void setEnabled(std::string_view name, bool enabled);   // console "system enable/disable <name>"
    bool enabled(std::string_view name) const;
    std::span<const SystemStats> stats() const;             // metrics overlay, console "systems"
    void clearGameSystems();                                 // before unloading a game module
};
```

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError
- [ENG-44](ENG-44.md) — Logger and sinks (console, file, developer console)

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-47](ENG-47.md) — Metrics and engine.debug_overlay (ImGui, ImPlot)
- [ENG-50](ENG-50.md) — Systems: previous_position, movement, hierarchy, parallax, lifetime
- [ENG-55](ENG-55.md) — Unit tests: scheduler, events, input, scenes, session
- [SRV-11](SRV-11.md) — RoomRuntime and RoomMailbox: one isolated game, lobby ↔ room messages

## Read first

- [ADR 0024](../adr/0024-error-handling-policy.md)
- [ADR 0028](../adr/0028-named-ordered-systems.md)
- [ADR 0039](../adr/0039-room-api.md)
- [reference 05](../engine/reference/05-scheduler.md)

## To implement

- [ ] `add(stage, name, fn)` + `SystemBuilder` (after, before, roles, disabled, critical)
- [ ] `build`: topological sort per stage; unknown name or cycle → error
- [ ] `run`: time each system, catch exceptions, apply the policy per role
- [ ] `setEnabled`, `stats`, `clearGameSystems`
- [ ] Per-room instances: `add<F>` keeps a copyable prototype, `addFactory`, `instantiate(stage)` (ADR 0039)

## Done when

- Tests: ordering, cycle message, server disables a throwing system, critical ends the session, client quits with code 5

## Watch out

- Ties keep registration order: deterministic

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
