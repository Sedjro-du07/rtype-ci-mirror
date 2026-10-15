# SRV-20 · Network measurements: snapshot sizes, bandwidth under netem (5 KB/s), reliable latency

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **04 Tests** · **Wave 17** · status and owner live on the board.

## Goal

Give the numbers the subject asks for: bandwidth and responsiveness under lag (network step 7).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `benchmarks/net/`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `benchmarks/net/`

```text
benchmarks/net/  SnapshotSizeBench (bytes per entity and per snapshot for R-Type rooms of 50 / 100 / 200 entities),
                 BandwidthUnderNetemBench (scripts/netem.sh: 150 ms, 2 % loss, duplicates, 5 KB/s; clumsy on Windows),
                 ReliableLatencyBench (delivery time under 5 % and 20 % loss)
                 — results written to docs/network/07-measurements.md
```

## Depends on

- [SRV-16](SRV-16.md) — Reconnection: grace period, single-use tokens, client auto-reconnect, UDP relink
- [SRV-17](SRV-17.md) — Replication v1: send until acknowledged, budget, NetPriority, relevance, quantize

## Unlocks

- no task waits for this one

## Read first

- [network step 1](../network/01-scope.md)
- [network step 7](../network/07-prototypes.md)

## To implement

- [ ] Snapshot size per entity and per room (50, 100, 200 entities)
- [ ] Bandwidth and playability under netem: 150 ms, 2 % loss, duplicates, 5 KB/s (clumsy on Windows)
- [ ] Reliable delivery time under 5 % and 20 % loss
- [ ] Write the results in `docs/network/07-measurements.md` and compare with the step 1 budgets

## Done when

- Measurements written; budgets of step 1 confirmed or corrected

## Watch out

- Measure the R-Type game, not a synthetic world

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
