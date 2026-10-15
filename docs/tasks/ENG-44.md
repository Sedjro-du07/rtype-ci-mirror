# ENG-44 · Logger and sinks (console, file, developer console)

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **12 Services** · **Wave 2** · status and owner live on the board.

## Goal

Thread-safe logging with tags and several sinks.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/services/Logger.hpp`
- `src/services/Logger.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/services/Logger.hpp`

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

### `src/services/Logger.cpp`

```cpp
class ConsoleSink final : public ILogSink {          // coloured stdout (Info and below) / stderr (Warn, Error)
public:
    explicit ConsoleSink(bool color);
    void write(const LogRecord& r) override;
    void flush() override;
};
class FileSink final : public ILogSink {             // rotating file
public:
    FileSink(std::filesystem::path file, size_t maxBytes, int keepFiles);
    void write(const LogRecord& r) override;
    void flush() override;
};
class ConsoleUiSink final : public ILogSink {        // feeds the developer console
public:
    explicit ConsoleUiSink(Console& console);
    void write(const LogRecord& r) override;
};
```

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError

## Unlocks

- [ENG-14](ENG-14.md) — ModuleManager: load by kind and name, version check, reverse unload
- [ENG-26](ENG-26.md) — Scheduler: stages, names, after/before, roles, stats, failure policy
- [SRV-09](SRV-09.md) — Contexts: ServiceContext, LobbyContext, one Context per room

## Read first

- [ADR 0035](../adr/0035-services-and-tools.md)
- [reference 12](../engine/reference/12-services.md)

## To implement

- [ ] Levels, `std::format` API, `tagged`
- [ ] ConsoleSink (colours), FileSink (rotating), ConsoleUiSink

## Done when

- Logs from the network thread and the game thread never interleave mid-line

## Watch out

- Never a singleton

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
