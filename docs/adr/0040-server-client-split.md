# ADR 0040: One game code, two builds; server and client folders in the engine and the game

- Status: Accepted
- Date: 2026-10-14
- Deciders: the R-Type team
- Discussion id: E-12 (moved to the Server part), SRV-10

## Context

A single game module loaded by both roles put client code (HUD, menus, effects) in the server and
server logic (AI, waves) in the client. The server also exists at two levels: the engine provides
server features, the game uses them; the engine's server features were spread over `net/`, `room/`
and `Engine.cpp`.

## Decision

1. **One game code, two builds (option C).** A game is organised in `shared/`, `server/` and `client/`.
   `engine_add_game_module(<name> SHARED_SOURCES … SERVER_SOURCES … CLIENT_SOURCES …)` builds
   `lib<name>_server` (shared + server) and `lib<name>_client` (shared + client). The engine still
   loads one game module per process: `ModuleManager::loadGame(name, role)` looks for
   `lib<name>_<role>`, then `lib<name>`. A simple game may give `SOURCES` only (one module for every role).
2. **The same split in the engine.** `include/engine/net/` holds what both sides use (transport, session,
   replication, `RoomTypes`, `RoomEvents`, `RoomNetwork`); `include/engine/server/` + `src/server/` hold
   the server features (`Lobby`, `RoomHandle`, `LobbyContext`, parties, rooms, room host, routing,
   `ServerLoop`); `include/engine/client/` + `src/client/` hold the client features (`ClientLobby`, the
   UI later). The server loop leaves `Engine.cpp` for `src/server/ServerLoop.cpp`.
3. **Inclusion rule.** Game `shared/` code includes the common engine headers and `engine/net/`; game
   `server/` code may also include `engine/server/`; game `client/` code may also include
   `engine/client/`. One engine library in Part 1.

## Alternatives considered

- **One module for both roles (A):** client code in the server, server logic in the client.
- **Three modules loaded together (B):** the engine would load several interdependent game modules in
  order; no known engine does it.

Industry: Source builds `server.dll` and `client.dll` from shared code compiled into both; Unreal and
Unity strip client code from server builds with build targets and defines.

## Consequences

- Client and server built from different shared code are refused at the handshake by the format
  fingerprint (ADR 0037).
- Room files moved from `room/` to `net/`, `server/` and `client/`; tasks SRV-08 to SRV-15 keep their
  content with new paths.
