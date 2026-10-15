# ADR 0012: An entity is a 64-bit index and generation

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: ECS-1

## Context

A plain `size_t` id is ambiguous once slots are reused: a stale id silently points at a new entity.

## Decision

`struct Entity { uint32_t index; uint32_t generation; }`. A reused slot gets a new generation; `alive(e)` compares two numbers. The handle never leaves the process.

## Alternatives considered

- **32-bit packed (20 + 12 bits, EnTT style):** compact, but caps entity count and wraps generations after 4096 reuses.

## Consequences

- Eight bytes per handle, irrelevant at our scale.
- Network identity is a separate question (deferred).
