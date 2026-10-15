# ADR 0011: The server is the engine in a generic server role

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-13

## Context

The team asked whether the server is part of the engine or uses it. Quake, Source and Unreal all run the server as the engine in a role, plus the game's server rules.

## Decision

Test for each feature: would an online Pong need it? Yes → engine: transport, connections, timeouts and crash notification, packet validation, network thread, replication, server role. R-Type only → game module: server-side rules, which components replicate. Deployment → application `r-type_server`: launcher and configuration. Lobby placement is decided in the Server part.

## Alternatives considered

- **Server inside the engine, lobby and rooms included:** game-specific orchestration would leak into the engine.
- **Server as a plain user of a socket-only engine:** all network work would be redone for each game.

## Consequences

- A large part of networking becomes engine work, reused by the second game.
- Generic replication costs more than hand-written messages; decided in the Network part.
