# ADR 0023: Absent services are Null Objects

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 1

## Context

The server has no window, renderer or audio but runs the same loop and receives the same `Context`. Returning `nullptr` forces a check at every call; one forgotten check crashes the server, and only on the server.

## Decision

`Context::audio()` returns `IAudio&`, `NullAudio` on the server; the same for `NullRenderer` and `NullPlatform`. The choice is made once in `Engine::boot()`. Null Objects are used only where doing nothing is correct: server, headless CI tests, a client with no audio device. A client whose rendering plugin fails stops with a clear message. Each Null Object in use is logged once at startup.

## Alternatives considered

- **`nullptr` + checks:** one omission is a server crash.
- **`std::optional<IAudio&>`:** the same checks, more visible.
- **Throwing when absent:** a sound must not stop a server.

## Consequences

- Shared systems run unchanged in both roles.
- Headless client runs in CI become possible.
