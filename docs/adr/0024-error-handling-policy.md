# ADR 0024: Boot errors stop cleanly, system errors never stop the server

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 1

## Context

The subject requires a server that keeps running whatever happens, and clear behaviour on errors. Errors differ in kind: boot failures, failures inside a system, external signals.

## Decision

Boot errors (config, plugin, API version, game module, scene) are reported with a precise message and `run()` returns an exit code per stage: 1 config, 2 plugin, 3 game module, 4 scene. Expected failures use `std::expected` (ADR 0017). An exception thrown by a system is caught by the `Scheduler`: on the server the system is logged and disabled and the game continues; on the client the error is logged and the client exits cleanly. `SIGINT` / `SIGTERM` call `requestQuit()`: the frame finishes, shutdown runs, logs are flushed. No exception may leave an `extern "C"` function; exceptions across C++ interfaces between modules built with the same toolchain are allowed.

## Alternatives considered

- **Crash on any error:** violates the subject's robustness requirement.
- **Swallow every error silently:** hides bugs.

## Consequences

- Real crashes (invalid pointer) cannot be caught: the architecture rules exist to prevent them.
- The server's policy when a critical system is disabled is refined in the Server part.
