# ADR 0006: Rendering goes through a per-frame command list

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-07

## Context

ECS storage order is not draw order, each draw call across a `.so` boundary is a virtual call, and two backends (SFML, raylib) must consume the same requests.

## Decision

Render systems fill a `RenderQueue` of plain `DrawCmd` structs (kind, texture handle, source rect, position, rotation, scale, tint, layer). The engine calls `IRenderer::submit()` once per frame; the backend sorts by layer, batches by texture and draws. The core only uses engine-owned handles and types.

## Alternatives considered

- **Immediate calls (`drawSprite` per entity):** simpler, but no global ordering, no batching, every feature changes both backends.

## Consequences

- Free `NullRenderer` for headless CI and a recording renderer for tests.
- 3D is an extra command kind behind `supports(Feature::Render3D)`.
- Dear ImGui stays outside the queue, drawn by the plugin after `submit()`.
