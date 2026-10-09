# ADR 0016: Entity identity is a set of optional components

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: ECS-5

## Context

The slides attach a fixed `EntityInfo` (uuid, name, tags, parent) to every entity.

## Decision

`Name`, `Uuid`, `Parent`, `Children` are ordinary optional components. Tags are either typed (empty components, fast queries) or text (`Tags` with interned ids) for YAML and scripts.

## Alternatives considered

- **Fixed EntityInfo card:** every missile and star pays for a string; special cases in serialization and the editor.

## Consequences

- One code path for serialization, inspector, scripts and network.
- Text tags let designers label entities without recompiling.
