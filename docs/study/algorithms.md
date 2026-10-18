# Algorithms

The algorithms that run every tick, what we compared and why we kept each one.

## ECS storage

| Option | Walk one type | Walk several types | Add / remove | Dead entities |
|---|---|---|---|---|
| **Paged sparse set** (chosen) | packed | walk the smallest set, look the others up | O(1), swap-and-pop | no cost |
| Sparse array (`vector<optional<T>>`) | every slot, holes included | same | fastest | 14× to 80× slower walks |
| Archetypes (one table per component set) | packed | 4× to 5× faster | moves every component of the entity | no cost |

Measured over 10 000 entities: the movement system takes **4 µs** with sparse sets, 0.025% of a
60 Hz frame. Archetypes would save 3 µs per system and cost a table migration on every added or
removed component, which R-Type does constantly (shields, power-ups, bullets). Details, method and
reproduction: [01 · ECS storage](01-ecs-storage.md); decision: [ADR 0013](../adr/0013-sparse-set-storage.md).

## Game loop

| Option | Verdict | Why |
|---|---|---|
| **fixed time step + accumulator, interpolated render** | **chosen** | the simulation runs at `tickRate` (60 Hz) whatever the frame rate; same result on client and server; `alpha` interpolates sprites between two ticks |
| variable step (`dt` = frame time) | rejected | physics depends on frame rate, client and server drift apart |
| lock to vsync | rejected | breaks on 144 Hz screens and on the headless server |

A frame longer than 0.25 s is clamped so a pause in a debugger does not run thousands of ticks
([ADR 0003](../adr/0003-fixed-timestep-loop.md)). The server sleeps until the next tick.

## System ordering

Systems are named (`engine.movement`, `rtype.bydos_ai`) and declare `after` / `before`. At boot each
stage is sorted with **Kahn's topological sort**; among systems with no constraint between them,
registration order wins, so the result is stable from run to run. An unknown name or a cycle is a
boot error with the names in the message.

| Option | Verdict | Why |
|---|---|---|
| **named systems + topological sort** | **chosen** | a game inserts its systems between engine ones without knowing their index |
| numeric priorities | rejected | magic numbers that collide between modules |
| registration order only | rejected | the game cannot place a system before an engine one |
| parallel job graph | rejected for Part 1 | single-threaded is simpler to debug; rooms run in parallel instead |

Record: [ADR 0028](../adr/0028-named-ordered-systems.md). Disabling a system by name is part of the
same design.

## Collision broad phase

| Option | Cost per tick (n colliders) | Verdict |
|---|---|---|
| all pairs | n² / 2 tests | rejected: 500 bullets → 125 000 tests |
| **uniform grid** (chosen) | about n × neighbours | cells of `gridCell` (64 px); each collider goes in the cells it covers; only pairs sharing a cell are tested, once |
| quadtree | n log n | better for very uneven sizes; R-Type objects are of similar size |
| sweep and prune | n log n | best when objects move little; R-Type bullets move fast |

The narrow phase is an AABB test, filtered by `layer` / `mask` bits before any geometry. Each pair
produces one `Collision` event. A grid is the simplest structure that removes the n² cost at
R-Type's scale.

## Events

Events are **queued** and dispatched after each tick, never inside a system
([ADR 0029](../adr/0029-queued-event-bus.md)). Immediate callbacks were rejected: a handler that
destroys an entity while a view walks it is a classic crash.

## Replication

| Step | Algorithm | Alternative rejected |
|---|---|---|
| identity | monotonic `NetworkId`, never reused, mapped per side | sending local `Entity` handles (they differ per process) |
| what to send | today full state at `snapshotRate`; SRV-17 sends only what is not yet acknowledged, per component byte hash | delta against the last *sent* state (breaks on loss) |
| ordering | newest tick per entity wins; destroyed ids remembered | sequence per message (a lost packet would block) |
| smoothing | client draws others `interpolationDelay` (0.1 s) in the past, linear between two states | extrapolation (overshoots on turns) |
| inputs | action bit mask, newest + 3 previous in every packet, server applies one per tick | sending positions from the client (cheating) |
| budget | entries sorted by accumulated `NetPriority`, cut at `sendBudgetBytesPerSecond` (SRV-17) | sending everything every tick |

Every message fits in 1024 bytes; snapshots are split into self-contained parts. Specification:
[RFC §9](../network/rfc.md); split between engine and game: [ADR 0041](../adr/0041-netcode-means-in-engine-policy-in-game.md).
