# ADR 0029: Events are typed, queued and unsubscribed by RAII

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 6

## Context

Systems must react to each other without knowing each other, and a reaction must never run in the middle of another system's iteration.

## Decision

`EventBus` with typed events (`ENGINE_EVENT`), `publish` queued until the next dispatch point (after each fixed step and after LateUpdate), `emit` immediate for engine internals, `publishFromAnyThread` through a thread-safe inbox for the network thread. `subscribe` returns a `Subscription` that unsubscribes on destruction. Events published during dispatch wait for the next one. A fixed set of engine events is documented.

## Alternatives considered

- **Immediate delivery only:** handlers would see half-updated worlds.
- **Priorities and request/answer messages:** not needed by any current use.

## Consequences

- No dangling callbacks after a listener is destroyed.
- Game subscriptions are cleared before the game module unloads.
