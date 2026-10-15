# ADR 0008: One audio plugin shared by both platform backends

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-09

## Context

SFML 3 and raylib both use miniaudio internally, and audio does not depend on the window.

## Decision

A single `audio_miniaudio` plugin serves both platform backends. `NullAudio` is used on the server and when no output device exists.

## Alternatives considered

- **Audio inside each platform plugin:** the same library wrapped twice for no benefit.

## Consequences

- miniaudio becomes a direct dependency through the package manager.
- Positional sound is written once.
