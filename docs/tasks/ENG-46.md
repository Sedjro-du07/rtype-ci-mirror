# ENG-46 · Console and built-in commands

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **12 Services** · **Wave 6** · status and owner live on the board.

## Goal

The developer console and its built-in commands.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/services/Console.hpp`
- `src/services/Console.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/services/Console.hpp`

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

## Depends on

- [ENG-45](ENG-45.md) — CVars: typed variables, flags, ranges, archive
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy

## Read first

- [ADR 0035](../adr/0035-services-and-tools.md)
- [guide 13](../engine/guide/13-console-cvars-metrics.md)

## To implement

- [ ] registerCommand, execute, print, history, complete, toggle
- [ ] Built-ins: help, set, get, cvars, quit, systems, system enable/disable, scene, entities, inspect, modules, net, bind, assets

## Done when

- Every built-in works headless through `execute`

## Watch out

- Cheat commands refused on a server unless `sv_cheats 1`

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
