# SRV-18 · Fuzz tests: UDP packets, TCP frames, handshake, snapshot and input decoding

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **04 Tests** · **Wave 17** · status and owner live on the board.

## Goal

Prove that no input can crash or exhaust the server (network step 7).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `tests/net/fuzz/`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `tests/net/fuzz/`

```text
tests/net/fuzz/  UdpPacketFuzzTest (random and mutated valid packets: header, every message id),
                 TcpStreamFuzzTest (random frames, split reads, huge lengths),
                 HandshakeFuzzTest, SnapshotDecodeFuzzTest, InputDecodeFuzzTest
                 — 100 000 inputs each, ASan + UBSan; seed corpus = the RFC §15 examples;
                 property: never crashes, never allocates beyond the limits, never reaches game code
```

## Depends on

- [SRV-07](SRV-07.md) — Tests: players, handshake, messages, codec, malformed input
- [SRV-17](SRV-17.md) — Replication v1: send until acknowledged, budget, NetPriority, relevance, quantize

## Unlocks

- no task waits for this one

## Read first

- [RFC §4.6, §11](../network/rfc.md) (MSEP v1.0)
- [network step 7](../network/07-prototypes.md)

## To implement

- [ ] Fuzzers for UDP packets, TCP frames, Hello, Snapshot and Input decoding
- [ ] Seed corpus from the RFC §15 examples, plus mutations
- [ ] Run under ASan and UBSan

## Done when

- 100 000 inputs per fuzzer, no crash, no allocation beyond the limits, nothing reaches game code

## Watch out

- Fuzz the real decoding path (`poll`), not a copy of it

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
