# 5 · Scheduler

Status: **Locked**. ADRs 0003, 0024, 0028.
Folder: `engine/include/engine/scheduler/`, `engine/src/scheduler/`.

## Role

The timetable of systems: which system runs in which stage, in which order, and what happens when
one fails. Single-threaded and deterministic by design.

---

## `enum class Stage` — `scheduler/Stage.hpp`

```cpp
enum class Stage : uint8_t {
    FixedUpdate,  // N times per frame at 1/tickRate: input, movement, AI, collisions, damage
    LateUpdate,   // once per frame after simulation: network send, hierarchy, cleanup
    Render        // once per frame, client and editor only: fill the RenderQueue
};
```

## `using SystemFn` and `enum RoleMask` — `scheduler/System.hpp`

```cpp
using SystemFn = std::move_only_function<void(Context&)>;
enum RoleMask : uint8_t { ClientRole = 1, ServerRole = 2, EditorRole = 4, AllRoles = 7 };
```

A system is any callable taking `Context&`: a free function, a lambda, or a functor with state.

## `class SystemBuilder` — `scheduler/Scheduler.hpp`

Returned by `Scheduler::add`, to declare ordering and options fluently.

```cpp
class SystemBuilder {
public:
    SystemBuilder& after(std::string_view other);   // run after this system (same stage)
    SystemBuilder& before(std::string_view other);  // run before this system (same stage)
    SystemBuilder& roles(uint8_t mask);             // only registered for these roles
    SystemBuilder& disabled();                      // starts disabled (enable from console or code)
    SystemBuilder& critical();                      // if it faults on the server, end the session cleanly
};
```

## `struct SystemStats` — `scheduler/Scheduler.hpp`

```cpp
struct SystemStats {
    std::string_view name;
    Stage            stage;
    double           lastMs, averageMs;  // timing for the profiler overlay
    bool             enabled;
    bool             faulted;            // threw and was disabled
    std::string      lastError;
};
```

## `class Scheduler` — `scheduler/Scheduler.hpp`

```cpp
class Scheduler {
public:
    explicit Scheduler(Role role, Logger& log);

    // Names are unique, "<module>.<system>": "engine.movement", "rtype.bydos_ai".
    SystemBuilder add(Stage stage, std::string name, SystemFn fn);

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

## Ordering rules

1. Engine systems are registered first with fixed names (block 13).
2. Within a stage, order follows `after` / `before`; ties keep registration order.
3. Ordering across stages is fixed by the loop and never declared.

## Failure policy (ADR 0024)

| Role | System throws | Result |
|---|---|---|
| Server | any system | logged with name and message, system disabled (`faulted = true`), game continues |
| Server | `critical()` system | logged, then the session ends cleanly: clients are told, `requestQuit()` |
| Client / Editor | any system | logged, error shown, `requestQuit()` |
