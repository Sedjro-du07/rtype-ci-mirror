# ENG-05 · Time and Clock

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **01 Core** · **Wave 1** · status and owner live on the board.

## Goal

Time values for the fixed-step loop and a monotonic clock to measure frames.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/core/Time.hpp`
- `src/core/Time.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/core/Time.hpp`

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

## Depends on

- nothing: can start now

## Unlocks

- no task waits for this one

## Read first

- [ADR 0003](../adr/0003-fixed-timestep-loop.md)
- [reference 01](../engine/reference/01-core.md)

## To implement

- [ ] `Time` struct (fixedDelta, tick, alpha, total, frameDelta)
- [ ] `Clock` over `std::chrono::steady_clock`: `restart`, `elapsed`

## Done when

- Test: `restart()` returns increasing positive values; `elapsed()` does not reset

## Watch out

- Never `system_clock` for durations: it jumps with NTP

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
