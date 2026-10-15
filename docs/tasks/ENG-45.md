# ENG-45 · CVars: typed variables, flags, ranges, archive

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **12 Services** · **Wave 5** · status and owner live on the board.

## Goal

Typed console variables with flags, ranges and persistence.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/services/CVars.hpp`
- `src/services/CVars.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/services/CVars.hpp`

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

struct CVarInfo {                      // read-only view for the console and the archive
    std::string_view name, description;
    uint32_t         flags;
    std::string      value;            // current value as text
    std::string      defaultValue;
};
```

## Depends on

- [ENG-06](ENG-06.md) — Signal and RAII Subscription
- [ENG-23](ENG-23.md) — YAML wrappers and field codecs
- [ENG-60](ENG-60.md) — OS layer: secure random, user data folder, executable folder, quit signals

## Unlocks

- [ENG-46](ENG-46.md) — Console and built-in commands
- [SRV-09](SRV-09.md) — Contexts: ServiceContext, LobbyContext, one Context per room

## Read first

- [ADR 0035](../adr/0035-services-and-tools.md)
- [ADR 0044](../adr/0044-platforms-and-portability.md)

## To implement

- [ ] `CVar<T>` with onChange
- [ ] `CVars::define`, set, get, find, forEach
- [ ] `loadArchive` / `saveArchive` (Archive flag) in `os::userDataDir()/settings.yaml` (ADR 0044)
- [ ] Engine CVars list of reference 12

## Done when

- Tests: range check, archive round trip

## Watch out

- `sim_tick_rate` is read-only

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
