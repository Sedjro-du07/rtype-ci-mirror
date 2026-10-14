# Network part

Designing the protocol RFC, step by step. **Done: MSEP v1.0 is frozen** ([rfc.md](rfc.md)).

| Step | Page | Status |
|---|---|---|
| 1 · Scope: already decided, subject requirements, budgets, threats | [01-scope.md](01-scope.md) | validated |
| 2 · Inventory of exchanges, as sequence diagrams, before any byte | [02-exchanges.md](02-exchanges.md) | validated (7 groups, 37 questions) |
| 3 · Layers: frame, session, reliability, engine messages, game messages | [03-layers.md](03-layers.md) | validated |
| 4 · Design choices, one by one | [04-decisions.md](04-decisions.md) | done: D1–D36 (D1 by the team, D2–D36 delegated to the architect) |
| 5 · RFC written as decisions land (engine protocol, then R-Type messages) | [rfc.md](rfc.md) | v1.0 written; Part II (R-Type messages) provisional until the Game part |
| 6 · Validation on scenarios | [06-scenarios.md](06-scenarios.md) | done: 40 scenarios, 4 precisions added to the RFC |
| 7 · Prototypes: real sizes, parser fuzzing, Wireshark dissector | [07-prototypes.md](07-prototypes.md) | planned as tasks SRV-18, SRV-19, SRV-20 |
| 8 · Freeze v1.0 and link to the tasks | this page | done: tasks reference RFC sections |

Decisions taken during this part: [ADR 0041](../adr/0041-netcode-means-in-engine-policy-in-game.md)
(the engine gives the means, the game team optimizes), [ADR 0042](../adr/0042-admission-by-the-game.md)
(admission by the game), [ADR 0043](../adr/0043-protocol-v1-game-api.md) (what the game sees of
protocol v1).

## Freeze (step 8)

- **Version 1 (`0x01`) is frozen.** Any change to a byte layout increments the version; the preamble
  and the `Refused` frame never change (RFC §4.1).
- **Part II of the RFC** (R-Type's messages) is completed by the Game part; it changes the game's
  fingerprint, never the protocol version.
- **Tasks linked to the RFC:** ENG-22 (quantize), ENG-24 (byte order), ENG-36 (reasons, settings),
  ENG-38 (frame, presence, counters), ENG-39 (snapshot, input), SRV-01 (session messages), SRV-02
  (handshake, admission), SRV-03 (ids), SRV-04 (field encoding), SRV-05 (reliability), SRV-06 (TCP
  frames), SRV-07 (tests), SRV-13 and SRV-14 (lobby, rooms), and the new SRV-16 (reconnection),
  SRV-17 (replication v1), SRV-18 (fuzz), SRV-19 (Wireshark), SRV-20 (measurements).
