# 01 · ECS storage: sparse set vs sparse array vs archetypes

Task ENG-56 · decision: [ADR 0013](../adr/0013-sparse-set-storage.md) · code: `engine/benchmarks/ecs_storage/`

## Question

Every system walks components every tick (60 times per second), and entities gain and lose
components all the time (bullets, shields, power-ups). Which storage serves both best?

## The three candidates

| Storage | Layout | Find one component | Walk a component type | Add / remove |
|---|---|---|---|---|
| **Paged sparse set** (`SparseSet<T>`, the engine's) | per type: packed `dense` entities + packed `data`, and a paged index → position table | two array reads | packed, no holes | O(1), swap-and-pop |
| **Sparse array** (`SparseArray<T>`) | per type: `std::vector<std::optional<T>>` indexed by entity | one array read | walks every slot, holes included | O(1), set or reset the slot |
| **Archetypes** (`MinimalArchetypeStore`) | one table per exact set of components, one column per component | location table + row | packed per table, all columns aligned | moves the entity to another table, copying all its components |

The archetype store is minimal (three fixed component types, eight tables): enough to measure, not
a production design.

## Scenario

- 10 000 entities. Every entity has a `Position` (8 bytes); one in two has a `Velocity` (8 bytes); one
  in three has a `Health` (4 bytes).
- **Spread 1**: entity indices 0 to 9 999 (a fresh level). **Spread 10**: indices 0, 10, 20, …
  99 990, as after many entities have died and only one slot in ten is alive.

| Benchmark | Game equivalent | Work |
|---|---|---|
| Iterate 1 | a system reading one component | `position.x += 1` for every `Position` |
| Iterate 2 | the movement system | `position += velocity * dt` for every entity with both (5 000) |
| Iterate 3 | a system reading three components | same, only when `health > 0` (1 667 entities) |
| Add/remove | shields or power-ups coming and going | add a `Health` to 10 000 entities, then remove it |

## Method

- Google Benchmark 1.9.5, `linux-gcc-release` (GCC 14.2, `-O3 -DNDEBUG`), Ubuntu 24.04.
- Intel Core i7-12700H, 20 threads, up to 4.7 GHz. CPU frequency scaling was on: absolute numbers
  move by a few percent between runs, ratios do not.
- 5 repetitions of each benchmark; the table shows the mean.
- Reproduce:

```sh
cmake --preset linux-gcc-release -DENGINE_BUILD_BENCHMARKS=ON
cmake --build --preset linux-gcc-release
./build/linux-gcc-release/bin/engine_bench_ecs_storage --benchmark_repetitions=5 --benchmark_report_aggregates_only=true
```

## Results

Time for one pass, in microseconds (lower is better).

| Benchmark | Spread | Sparse set | Sparse array | Archetypes |
|---|---|---|---|---|
| Iterate 1 (10 000) | 1 | 1.63 | 2.99 | 1.66 |
| Iterate 1 (10 000) | 10 | 1.65 | 22.33 | 1.65 |
| Iterate 2 (5 000) | 1 | 3.63 | 4.14 | 0.93 |
| Iterate 2 (5 000) | 10 | 4.10 | 35.37 | 0.89 |
| Iterate 3 (1 667) | 1 | 3.90 | 4.43 | 0.85 |
| Iterate 3 (1 667) | 10 | 4.93 | 73.02 | 0.88 |
| Add + remove (20 000 operations) | 1 | 208.76 | 8.30 | 127.85 |
| Add + remove (20 000 operations) | 10 | 247.38 | 18.11 | 150.39 |

## Reading

**Sparse array.** Fastest to add and remove (setting an `optional` is all it does), but every walk
visits every slot. In a fresh level it is already 2× slower than the packed storages; once entities
have died (spread 10) it is 14× to 80× slower. A long R-Type level kills thousands of entities, so
this is the case that matters. Rejected, as in ADR 0013.

**Archetypes.** As fast as the sparse set for one component, and **4× to 5× faster for two or three
components**: the columns of a table line up, while the sparse set walks the smallest set and looks
the other components up by entity, one random read each. In this test archetypes are also
**1.6× faster than the sparse set to add and remove**, which contradicts the ADR's expectation.
The reason: our entities carry only two small components (16 bytes) that move with them. A real
R-Type entity carries eight or more (`Position`, `Velocity`, `Sprite`, `Collider`, `Health`,
`NetworkId`, `Lifetime`, …), some of them larger, and every archetype change copies all of them,
while a sparse set only touches the one component added or removed. This benchmark does not measure
that case yet.

**Sparse set.** Never the slowest at walking, never affected by dead entities (spread 10 costs at
most 25%), and its add/remove cost does not depend on what else the entity carries. Its
add/remove is the slowest here: `remove` looks the page table up four times
(`has`, `position`, two `slot`); keeping the position found by `has` would remove two of them.

## Conclusion

The sparse set stays (ADR 0013), for these reasons:

- **Fast enough by a wide margin.** The movement system over 5 000 entities takes 4 µs, 0.025% of a
  60 Hz frame (16 667 µs). Archetypes would save 3 µs per system per frame.
- **Predictable.** Walking cost does not degrade as entities die, and adding or removing a component
  costs the same whatever the entity carries.
- **Simpler.** One container per component type, no table migration, no query cache: easier to write,
  test and explain, which the subject values.

Archetypes are the better choice for engines whose bottleneck is iterating many components over
hundreds of thousands of entities; that is not R-Type's scale. If profiling ever shows ECS walks as
a cost, `View` (ENG-19) is the place to optimise, without changing storages.

## Follow-ups

- Measure add/remove with heavier entities (8 components, 100+ bytes) to check the archetype copy
  cost claimed above.
- Tighten `SparseSet<T>::remove` (reuse the position found by `has`) and measure again.
