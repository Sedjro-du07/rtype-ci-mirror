# ADR 0034: One active scene, prefabs with overrides, handle-based assets

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 11

## Context

Scenes and prefabs come from YAML (ADR 0018); the slides propose one registry per scene and a two-pass loader.

## Decision

One active scene, owning one registry. `SceneLoader` parses in two passes (create with prefab and overrides, then link by uuid), reports `file:line`, and leaves no partial scene on error. `SceneManager` switches at the end of a frame, keeps entities marked persistent, keeps the current scene if the next one fails, and saves only what differs from prefabs. `PrefabLibrary` caches parsed prefabs and instantiates them with children. `AssetManager` maps paths to handles, loads once, reference-counts, refuses paths outside the game root, falls back to a visible missing texture, and hot-reloads in development.

## Alternatives considered

- **Several scenes loaded at once:** not needed by R-Type or Pong; adds cross-scene references.

## Consequences

- Asynchronous loading screens are a Part 2 addition behind the same API.
