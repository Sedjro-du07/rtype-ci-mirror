# ENG-60 · OS layer: secure random, user data folder, executable folder, quit signals

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **01 Core** · **Wave 2** · status and owner live on the board.

## Goal

Keep every operating-system call in one private place, so the rest of the engine is the same on Linux and Windows.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/os/Os.hpp`
- `src/os/Os.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `src/os/Os.hpp`

```cpp
// Private: the only engine code with #ifdef _WIN32 besides DynamicLibrary (ADR 0044).
namespace engine::os {
void                  secureRandom(std::span<std::byte> out);            // getrandom / BCryptGenRandom; aborts on failure
std::filesystem::path userDataDir(std::string_view app);                  // created if missing
std::filesystem::path executableDir();
void                  onQuitSignal(std::move_only_function<void()> fn);  // SIGINT, SIGTERM / console control
}
```

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError

## Unlocks

- [ENG-10](ENG-10.md) — Generic launcher main() and signal handling
- [ENG-45](ENG-45.md) — CVars: typed variables, flags, ranges, archive
- [SRV-02](SRV-02.md) — Handshake: Hello checks, game admission, Challenge / UdpLink, Accepted / Refused, flood limits

## Read first

- [ADR 0044](../adr/0044-platforms-and-portability.md)
- [reference 01](../engine/reference/01-core.md)

## To implement

- [ ] `os::secureRandom(span<byte>)`: `getrandom` on Linux, `BCryptGenRandom` on Windows (MSEP tokens and keys)
- [ ] `os::userDataDir(app)`: `$XDG_DATA_HOME` or `~/.local/share` / `%APPDATA%`, created on demand
- [ ] `os::executableDir()`: `/proc/self/exe` / `GetModuleFileNameW`
- [ ] `os::onQuitSignal(fn)`: SIGINT and SIGTERM / `SetConsoleCtrlHandler`; the callback only sets an atomic

## Done when

- Tests: 1 000 random draws all differ; the user folder exists after the call; the executable folder contains the test binary

## Watch out

- `#ifdef _WIN32` is allowed only here, in `DynamicLibrary` and in plugins (ADR 0044)
- `NOMINMAX` and `WIN32_LEAN_AND_MEAN` before `<windows.h>`, inside `.cpp` files only

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
