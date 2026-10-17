# ENG-47 · Metrics and engine.debug_overlay (ImGui, ImPlot)

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **12 Services** · **Wave 8** · status and owner live on the board.

## Goal

Metrics and the debug overlay.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/services/Metrics.hpp`
- `src/services/Metrics.cpp`
- `src/systems/DebugOverlaySystem.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/services/Metrics.hpp`

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

struct MetricsSnapshot {
    double   frameMs, fps;
    std::vector<std::pair<std::string_view, double>> systemMs;   // per system, last frame
    size_t   entities;
    uint32_t drawCalls;
    double   rttMs, lossPercent, kbIn, kbOut;                     // network, 0 offline
};
```

### `src/systems/DebugOverlaySystem.cpp`

```cpp
void debugOverlay(Context& ctx);   // "engine.debug_overlay": Render, client and editor, last
// private helpers
void drawConsole(Console& console);
void drawFrameGraph(const Metrics& metrics);
void drawSystemTimings(std::span<const SystemStats> stats);
void drawLagometer(const NetworkSession& net);
```

## Depends on

- [ENG-32](ENG-32.md) — Dear ImGui and ImPlot in platform_sfml (debug UI hooks)
- [ENG-26](ENG-26.md) — Scheduler: stages, names, after/before, roles, stats, failure policy

## Unlocks

- [SRV-09](SRV-09.md) — Contexts: ServiceContext, LobbyContext, one Context per room

## Read first

- [ADR 0035](../adr/0035-services-and-tools.md)

## To implement

- [ ] `Metrics`: frame timing, series ring buffers, snapshot
- [ ] `engine.debug_overlay`: console, FPS graph, system timings, lagometer

## Done when

- Overlay shows per-system times matching `systems`

## Watch out

- Recording must stay cheap: no allocation per sample

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
