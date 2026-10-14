# 1 · Engine, Context, lifecycle, core types

Status: **Locked**. ADRs 0003, 0005, 0017, 0021, 0022, 0023, 0024.
Folder: `engine/include/engine/` (`Engine.hpp`, `Context.hpp`, `EngineConfig.hpp`),
`engine/include/engine/core/` (Time, Error, math, Signal, hashing), `engine/src/core/`, `engine/launcher/`.

## Role

`Engine` is the composition root: it owns every subsystem, runs the lifecycle and the game loop,
loads plugins and the game module. Game code never sees it; it receives a `Context`.

---

## `enum class Role` — `EngineConfig.hpp`

```cpp
enum class Role : uint8_t { Client, Server, Editor };
```

The role of the process. Decides which plugins load (ADR 0004) and is passed to
`IGameModule::registerSystems`.

## `struct EngineConfig` — `EngineConfig.hpp`

```cpp
struct EngineConfig {
    Role                  role = Role::Client;
    std::string           game;             // game module name: "rtype" → games/librtype.so
    std::string           platform = "sfml";// platform plugin (client, editor)
    std::string           audio = "miniaudio";
    std::string           network = "asio";
    std::string           scripting = "lua";// empty string = no scripting plugin (NullScripting)
    double                tickRate = 60.0;  // fixed simulation steps per second (ADR 0003)
    std::filesystem::path root;             // folder of the game: assets/, scenes/, prefabs/
    WindowDesc            window;           // client only, see block 8
    NetConfig             net;              // see block 10
    LogConfig             log;              // see block 12
    YAML::Node            cvars;            // initial console variables, see block 12
    std::filesystem::path sceneOverride;    // --scene: start here instead of IGameModule::firstScene()
    uint64_t              maxTicks = 0;     // --ticks: quit cleanly after N steps (0 = never); tests and CI

    // Reads the YAML file, then applies command-line overrides (--config, --role, --platform, --audio,
    // --network, --scripting, --port, --host, --log, --scene, --ticks, --set name=value). Unknown keys are an error with file:line.
    static Result<EngineConfig> load(const std::filesystem::path& file, int argc, char** argv);
};
```

`YAML::Node` appears only in `EngineConfig.hpp`, which game code never includes.

## `struct Time` and `class Clock` — `core/Time.hpp`

```cpp
struct Time {
    double   fixedDelta = 1.0 / 60.0; // seconds per FixedUpdate step; the dt every simulation system uses
    uint64_t tick = 0;                // number of fixed steps since start
    double   alpha = 0.0;             // [0,1): position between the last two ticks, for interpolation
    double   total = 0.0;             // seconds of simulated time since start
    double   frameDelta = 0.0;        // real seconds of the last frame (rendering, UI only)
};

class Clock {
public:
    Clock();                          // starts measuring
    double restart();                 // seconds since construction or last restart, then resets
    double elapsed() const;           // seconds since last restart, without resetting
};
```

## `struct Error`, `Result<T>`, `BootError` — `core/Error.hpp`

```cpp
enum class Errc : uint16_t {
    ConfigInvalid, FileNotFound, ParseError, PluginNotFound, SymbolNotFound, ApiMismatch,
    GameModuleInvalid, UnknownComponent, UnknownPrefab, DuplicateName, ScheduleCycle,
    AssetLoadFailed, ScriptError, NetworkError, InvalidArgument
};

struct Error {
    Errc        code;
    std::string message;       // human readable, complete sentence
    std::string file;          // optional: source file of the error (YAML, script)
    int         line = 0;      // optional: 1-based line in that file
    std::string toString() const; // "scenes/level1.yaml:14: unknown component 'Helth' (did you mean 'Health'?)"
};

template <class T> using Result = std::expected<T, Error>;

class BootError : public std::runtime_error {      // thrown only by boot() and load()
public:
    BootError(int exitCode, Error error);
    int exitCode() const noexcept;                 // 1 config, 2 plugin, 3 game module or schedule, 4 scene
                                                   // (5 = client stopped after a system error, returned by run())
    const Error& error() const noexcept;
};
```

## Math vocabulary — `core/math/`

```cpp
struct Vec2  { float x = 0, y = 0; /* +, -, * scalar, dot, length, normalized, lerp */ };
struct Rect  { float x = 0, y = 0, w = 0, h = 0; bool contains(Vec2) const; bool intersects(const Rect&) const; };
struct Color { uint8_t r = 255, g = 255, b = 255, a = 255; static constexpr Color white(); /* black, red… */ };
constexpr uint64_t fnv1a64(std::string_view) noexcept;   // core/Hash.hpp: compile-time hash for type, event and action keys
```

`Signal<void(Args...)>` and `Subscription` also live in `core/` (`core/Signal.hpp`); they are
documented with the events block (06-events) where they are used most.

---

## `class Engine` — `Engine.hpp`

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

### The loop (`Engine::loop`)

```
while running_:
    accumulator += clock_.restart()  (clamped to 0.25 s to avoid a spiral after a stall)
    platform.pollEvents(input_, events_)        raw input → InputMap; window events → EventBus
    network_.poll(ctx)                          network inbox → player events, on<T> message callbacks
    while accumulator >= step:
        input_.beginTick()                      latch edges for this tick
        scheduler_.run(FixedUpdate, ctx)
        commands_.apply(scenes_.world())        structural changes (ADR 0015), hooks fire
        events_.dispatch()
        accumulator -= step; ++time_.tick
    scheduler_.run(LateUpdate, ctx); commands_.apply(...); events_.dispatch()
    scenes_.applyPending(ctx)                   deferred scene switch
    time_.alpha = accumulator / step
    scheduler_.run(Render, ctx)                 render systems fill renderQueue_
    platform.renderer().submit(renderQueue_.frame()); renderQueue_.clear()
    audio.update(); metrics_.endFrame()
```

The server runs the same loop with Null platform, renderer and audio, and nothing in `Render`.

---

## `class Context` — `Context.hpp`

The only door from game code into the engine (ADR 0022). Non-owning, built only by `Engine`,
used only on the loop thread, valid only during the call it was passed to.

```cpp
class Context {
public:
    Registry&          world();             // registry of the ACTIVE scene; re-read it after a scene switch
    Commands&          commands();          // deferred spawn / destroy / add / remove
    SceneManager&      scenes();            // switchTo(), markPersistent(), active scene name
    EventBus&          events();            // publish / subscribe
    const ActionState& input() const;       // local player's actions this tick
    InputMap&          inputMap();          // bindings and options (options menu, rebinding)
    AssetManager&      assets();            // textures, sounds, music, fonts, scripts by path
    RenderQueue&       render();            // describe what to draw this frame
    IAudio&            audio();             // NullAudio on the server
    NetworkSession&    network();           // players, messages, stats (block 10)
    const Time&        time() const;        // fixedDelta, tick, alpha, total
    Logger&            log();
    CVars&             cvars();
    Console&           console();
    Metrics&           metrics();           // record your own series for the overlay
    Role               role() const;
    void               requestQuit();       // a request: the frame finishes cleanly

private:
    friend class Engine;
    explicit Context(Engine& engine);
    Engine* engine_;
};
```

---

## Launcher — `engine/launcher/main.cpp`

```cpp
int main(int argc, char** argv);
// Loads EngineConfig from ENGINE_DEFAULT_CONFIG (set by engine_add_launcher) and argv,
// installs SIGINT / SIGTERM (and the Windows console handler) to call Engine::requestQuit,
// then returns Engine::run(). Prints Error::toString() and returns 1 if the config is invalid.
```

Games never write `main()`; CMake builds this file once per executable (ADR 0005, block 2).
