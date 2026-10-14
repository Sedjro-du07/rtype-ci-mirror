# ENG-04 · Core types: hash, Vec2/Rect/Color, Error, Result, BootError

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **01 Core** · status and owner live on the board.

## Goal

Write the leaf vocabulary every block uses: hashing, math types, errors and results.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/core/Hash.hpp`
- `include/engine/core/math/Vec2.hpp`
- `include/engine/core/math/Rect.hpp`
- `include/engine/core/math/Color.hpp`
- `include/engine/core/Error.hpp`
- `src/core/Error.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/core/Error.hpp`

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

### `include/engine/core/Hash.hpp`

```cpp
constexpr uint64_t fnv1a64(std::string_view text) noexcept;   // compile-time hash for type, event and action keys
```

### `include/engine/core/math/Vec2.hpp`

```cpp
struct Vec2 {
    float x = 0, y = 0;
    constexpr Vec2 operator+(Vec2 o) const;   constexpr Vec2 operator-(Vec2 o) const;
    constexpr Vec2 operator*(float k) const;  constexpr Vec2 operator/(float k) const;
    constexpr Vec2& operator+=(Vec2 o);       constexpr Vec2& operator-=(Vec2 o);
    constexpr Vec2& operator*=(float k);
    constexpr bool operator==(const Vec2&) const = default;
    constexpr float dot(Vec2 o) const;
    float length() const;
    Vec2  normalized() const;                 // {0, 0} stays {0, 0}
    static constexpr Vec2 lerp(Vec2 a, Vec2 b, float t);
};
```

### `include/engine/core/math/Rect.hpp`

```cpp
struct Rect {
    float x = 0, y = 0, w = 0, h = 0;
    constexpr bool contains(Vec2 p) const;           // edges included on the left/top, excluded on the right/bottom
    constexpr bool intersects(const Rect& o) const;  // touching edges do not intersect
    constexpr Vec2 center() const;
    constexpr bool operator==(const Rect&) const = default;
};
```

### `include/engine/core/math/Color.hpp`

```cpp
struct Color {
    uint8_t r = 255, g = 255, b = 255, a = 255;
    static constexpr Color white(); static constexpr Color black();
    static constexpr Color red();   static constexpr Color green(); static constexpr Color blue();
    static constexpr Color transparent();
    constexpr bool operator==(const Color&) const = default;
};
```

## Depends on

- nothing: can start now

## Unlocks

- [ENG-07](ENG-07.md) — EngineConfig: YAML file and command-line overrides
- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE
- [ENG-12](ENG-12.md) — DynamicLibrary for Linux and Windows
- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)
- [ENG-24](ENG-24.md) — ByteWriter / ByteReader: bounds-checked, bit packing
- [ENG-26](ENG-26.md) — Scheduler: stages, names, after/before, roles, stats, failure policy
- [ENG-28](ENG-28.md) — Keys, RawInputEvent, ActionId
- [ENG-30](ENG-30.md) — Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform
- [ENG-44](ENG-44.md) — Logger and sinks (console, file, developer console)

## Read first

- [ADR 0017](../adr/0017-cpp23-with-allow-list.md)
- [ADR 0024](../adr/0024-error-handling-policy.md)
- [reference 01](../engine/reference/01-core.md)

## To implement

- [ ] `fnv1a64` as `constexpr` (usable in templates and macros)
- [ ] `Vec2` (operators, `dot`, `length`, `normalized`, `lerp`), `Rect` (`contains`, `intersects`), `Color` (named constants)
- [ ] `Errc` (15 codes), `Error` with `toString()` → `file:line: message`
- [ ] `Result<T>` alias, `BootError` with exit code

## Done when

- Unit tests: hash known values at compile time (`static_assert`), rect intersection edges, `Error::toString` with and without file

## Watch out

- No engine include in `core/`: it is the bottom layer
- `Error::message` is a full sentence; suggestions ("did you mean") are added by callers

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
