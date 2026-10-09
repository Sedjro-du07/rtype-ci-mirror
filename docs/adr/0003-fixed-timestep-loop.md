# ADR 0003: The simulation advances in fixed steps and rendering interpolates

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-04

## Context

The subject forbids tying game time to CPU speed. Multiplying by a variable delta time fixes speed but makes every machine compute slightly different results (tunnelling, divergence), which a networked game cannot accept.

## Decision

The loop accumulates real time and runs the simulation in fixed steps of `1 / tickRate` (60 Hz by default). Rendering runs as often as possible and draws positions interpolated with `alpha = accumulator / step`. The same loop runs on the client and the server.

## Alternatives considered

- **Variable delta time:** simple, but non-deterministic across machines.
- **One update per frame at a fixed amount:** speed depends on the frame rate, forbidden by the subject.

## Consequences

- Each moving entity keeps its previous state for interpolation.
- The same idea serves network snapshot interpolation later.
- `tickRate` lives in the configuration and must match on client and server.
