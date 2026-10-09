# ADR 0015: Structural changes go through a command buffer

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: ECS-4

## Context

Adding, removing or destroying during iteration breaks sparse-set iteration (swap-and-pop skips or repeats entities).

## Decision

Systems record spawn, destroy, add and remove in `Commands`; the engine applies them at the end of each stage, where hooks fire. Editing a component's value stays immediate.

## Alternatives considered

- **Nothing deferred:** fragile.
- **Only destruction deferred (slides):** add and remove remain unsafe.

## Consequences

- Deterministic order, useful for networking.
- The natural place for Observer hooks.
