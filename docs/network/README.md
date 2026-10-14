# Network part

Designing the protocol RFC, step by step. Each step is validated before the next one starts.

| Step | Page | Status |
|---|---|---|
| 1 · Scope: already decided, subject requirements, budgets, threats | [01-scope.md](01-scope.md) | validated |
| 2 · Inventory of exchanges, as sequence diagrams, before any byte | [02-exchanges.md](02-exchanges.md) | groups 1–4 of 7 validated |
| 3 · Layers: frame, session, reliability, engine messages, game messages | — | |
| 4 · Design choices, one by one | — | |
| 5 · RFC written as decisions land (engine protocol, then R-Type messages) | — | |
| 6 · Validation on scenarios (loss, reorder, duplicates, malformed, full, version, reconnection) | — | |
| 7 · Prototypes: real sizes, parser fuzzing, Wireshark dissector | — | |
| 8 · Freeze v1.0 and link to the tasks | — | |

Decisions taken during this part: [ADR 0041](../adr/0041-netcode-means-in-engine-policy-in-game.md) (the engine gives the means, the game team optimizes), [ADR 0042](../adr/0042-admission-by-the-game.md) (admission by the game).
