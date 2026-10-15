# 12 · Services, tools, scripting

Status: **Locked**. ADRs 0019, 0022, 0035.
Folder: `engine/include/engine/services/`, `engine/include/engine/script/`, `engine/src/services/`,
`engine/src/script/`, `engine/plugins/scripting_lua/`.

## Role

The services every module needs (logging, console variables, console, metrics), the developer
tools built on them, and the scripting interface for the content layer.

---

## Logging — `services/Logger.hpp`

```cpp
enum class LogLevel : uint8_t { Trace, Debug, Info, Warn, Error };

struct LogRecord { LogLevel level; std::string_view tag; std::string_view message;
                   std::chrono::system_clock::time_point time; };

class ILogSink {
public:
    virtual ~ILogSink() = default;
    virtual void write(const LogRecord&) = 0;
    virtual void flush();                       // default: does nothing
};
// Provided sinks: ConsoleSink (colored stdout/stderr), FileSink (rotating file), ConsoleUiSink (developer console)

struct LogConfig { LogLevel level = LogLevel::Info; std::filesystem::path file = "logs/engine.log"; bool color = true; };

class Logger {
public:
    explicit Logger(const LogConfig& config);
    template <class... A> void trace(std::format_string<A...>, A&&...);
    template <class... A> void debug(std::format_string<A...>, A&&...);
    template <class... A> void info (std::format_string<A...>, A&&...);
    template <class... A> void warn (std::format_string<A...>, A&&...);
    template <class... A> void error(std::format_string<A...>, A&&...);
    Logger tagged(std::string_view tag);           // child logger sharing sinks: "[platform_sfml] …"
    void   setLevel(LogLevel level);
    void   addSink(std::shared_ptr<ILogSink> sink);
    void   flush();                                 // called at shutdown and on errors
};
```

Thread-safe: the network thread may log. Never a singleton (ADR 0022).

## Console variables — `services/CVars.hpp`

```cpp
enum CVarFlags : uint32_t { CVarNone = 0, Archive = 1, Cheat = 2, ReadOnly = 4, ServerOnly = 8 };

template <class T> class CVar {               // T: bool, int, float, std::string
public:
    const T& value() const;
    void set(T v);                            // validates range, fires onChange
    [[nodiscard]] Subscription onChange(std::move_only_function<void(const T&)> fn);
};

class CVars {
public:
    template <class T> CVar<T>& define(std::string_view name, T defaultValue,
                                       std::string_view description, uint32_t flags = CVarNone,
                                       std::optional<std::pair<T, T>> range = {});
    Result<void> set(std::string_view name, std::string_view value);  // console and config
    std::optional<std::string> get(std::string_view name) const;
    template <class T> CVar<T>* find(std::string_view name);
    void forEach(std::move_only_function<void(const CVarInfo&)> fn) const;
    Result<void> loadArchive(const std::filesystem::path& file);      // user settings (volumes, bindings options)
    Result<void> saveArchive(const std::filesystem::path& file) const;// Archive-flagged only
};
```

Engine-defined variables include `r_vsync`, `r_ui_scale`, `r_color_filter`, `r_debug_colliders`,
`snd_master`, `snd_music`, `snd_effects`, `snd_interface`, `in_deadzone`, `net_snapshot_rate`,
`net_timeout`, `asset_hot_reload`, `phys_grid_cell`, `sim_tick_rate` (read-only).

The engine calls `loadArchive("user/settings.yaml")` after reading the config and `saveArchive` at shutdown,
so `Archive` variables (volumes, UI scale, colour filter, deadzone, game options) persist between runs.

## Console — `services/Console.hpp`

```cpp
using CommandFn = std::move_only_function<void(std::span<const std::string_view> args, Console&)>;

class Console {
public:
    Console(CVars& cvars, EventBus& events, Logger& log);
    void registerCommand(std::string_view name, std::string_view help, CommandFn fn);
    void execute(std::string_view line);      // "set r_vsync 0", "system disable rtype.bydos_ai"
    void print(std::string_view text);
    std::span<const std::string> history() const;
    std::span<const std::string> output() const;
    std::vector<std::string> complete(std::string_view prefix) const; // tab completion
    bool open() const; void toggle();          // F1 by default (bound in input.yaml)
};
```

Built-in commands: `help`, `set`, `get`, `cvars`, `quit`, `systems`, `system enable|disable <name>`,
`scene <path>`, `entities`, `inspect <name>`, `modules`, `net`, `bind <action> <key>`, `assets`.
Commands flagged `Cheat` are refused on a server unless `sv_cheats 1`.

## Metrics — `services/Metrics.hpp`

```cpp
class Metrics {
public:
    void beginFrame(); void endFrame();                     // engine only
    void record(std::string_view series, double value);    // any module: "net.rtt", "rtype.enemies"
    double fps() const; double frameMs() const;
    std::span<const double> history(std::string_view series) const; // ring buffer (300 samples) for ImPlot
    MetricsSnapshot snapshot() const;                       // frame time, per-system times, entity count,
                                                            // draw calls, network rtt / loss / bandwidth
};
```

## Debug overlay — `systems/DebugOverlaySystem.cpp`

Engine system `engine.debug_overlay` (Render, client and editor): draws the console, an FPS and
frame-time graph, per-system timings, entity counts and the network lagometer with Dear ImGui and
ImPlot through `IPlatform::beginDebugUi / endDebugUi`.

---

## Scripting — `script/IScripting.hpp`

```cpp
struct ScriptHost {                      // what the engine exposes to scripts (bound by the plugin)
    TypeRegistry&  types;                // components by name, through reflection
    AssetManager&  assets;
    EventBus&      events;
    Logger&        log;
};

class IScripting : public IPlugin {
public:
    virtual Result<void>          bind(ScriptHost& host) = 0;         // engine only, once after types are registered
    virtual Result<ScriptHandle>  load(const std::filesystem::path& file) = 0; // compile, sandboxed
    virtual void                  unload(ScriptHandle) = 0;
    virtual Result<void>          reload(ScriptHandle) = 0;           // keeps the handle, swaps the code
    virtual Result<void>          call(ScriptHandle s, std::string_view function,
                                       Entity self, Context& ctx) = 0; // on_spawn, on_update, on_collision…
    virtual bool                  has(ScriptHandle s, std::string_view function) const = 0;
};
```

### Component and system

```cpp
struct Script { ScriptHandle handle; std::string file; };   // Transient handle, file saved; ENGINE_COMPONENT
```

| System | Stage | Calls |
|---|---|---|
| `engine.script.lifecycle` | FixedUpdate | `on_spawn(self)` once, `on_destroy(self)` through the Script onDestroy hook |
| `engine.script.update` | FixedUpdate | `on_update(self, dt)` for every entity with `Script` |
| `engine.script.collision` | FixedUpdate | `on_collision(self, other)` from `Collision` events |

A script error is reported with `file:line`, the entity's script is disabled, the game continues.

### Plugin `scripting_lua`

Lua 5.4 + sol2. Sandbox: only `base` (minus `load`, `dofile`), `math`, `string`, `table`; no `io`,
`os`, `package`, `debug`. An instruction-count hook stops a script that runs too long in one call.
The Lua API (`self`, `ctx`, components by name, `spawn`, `destroy`, `play`, `publish`) is documented
in the guide (11-scripting-lua).
