# ENG-12 · DynamicLibrary for Linux and Windows

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **02 Modules** · **Wave 2** · status and owner live on the board.

## Goal

The only OS-specific loading code.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/module/DynamicLibrary.hpp`
- `src/module/DynamicLibrary.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/module/DynamicLibrary.hpp`

```cpp
class DynamicLibrary {
public:
    static Result<DynamicLibrary> open(const std::filesystem::path& file); // FileNotFound / SymbolNotFound with the OS message
    static std::string decorate(std::string_view stem);  // "platform_sfml" → "libplatform_sfml.so" / "platform_sfml.dll"
    DynamicLibrary(DynamicLibrary&&) noexcept; DynamicLibrary& operator=(DynamicLibrary&&) noexcept;
    ~DynamicLibrary();                                   // dlclose / FreeLibrary
    void* rawSymbol(std::string_view name) const;        // nullptr if absent
    template <class Fn> Result<Fn> symbol(std::string_view name) const; // typed lookup, SymbolNotFound if absent
    const std::filesystem::path& path() const;
};
```

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError

## Unlocks

- [ENG-14](ENG-14.md) — ModuleManager: load by kind and name, version check, reverse unload

## Read first

- [ADR 0002](../adr/0002-dynamic-plugins-from-day-one.md)
- [ADR 0025](../adr/0025-module-loading.md)

## To implement

- [ ] `open` (dlopen / LoadLibraryW) with the OS error text
- [ ] `symbol<Fn>` / `rawSymbol`
- [ ] `decorate(stem)` per OS
- [ ] RAII close, move-only

## Done when

- Tests: open missing file, missing symbol, valid fixture on Linux and Windows

## Watch out

- `RTLD_NOW | RTLD_LOCAL` on Linux; wide paths on Windows

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
