# ADR 0013: Components are stored in paged sparse sets

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: ECS-2

## Context

Sparse arrays (the slides) waste memory and iterate over holes; archetypes are fastest for queries but make adding and removing components expensive, and the slides encourage changing components at runtime.

## Decision

One paged sparse set per component type: a paged sparse array maps an entity to a dense position, dense arrays stay packed, add and remove are O(1) by swap-and-pop, iteration is contiguous, views walk the smallest set.

## Alternatives considered

- **Sparse array:** simplest, slow iteration, wasted memory.
- **Archetypes:** best multi-component queries, costly structural changes, much harder to write and explain.

## Consequences

- Dense order changes on removal: never keep a pointer to a component.
- A benchmark of the three goes into the comparative study: [01 · ECS storage](../study/01-ecs-storage.md)
  (ENG-56). Archetypes walk multi-component queries 4–5× faster; the sparse set is kept for
  predictable costs and simplicity, well within the frame budget.
