# ADR 0039: Room API: lobby, parties, rooms, contexts

- Status: Accepted
- Date: 2026-10-14
- Deciders: the R-Type team
- Discussion id: SRV-6 to SRV-9 (Server part, round 3)
- Builds on: [ADR 0038](0038-rooms-shared-host.md); amends [ADR 0022](0022-context-facade.md)

## Context

ADR 0038 fixed the principles of rooms. The game needs a concrete API, and the engine needs to know
which state becomes per room and which stays shared.

## Decision

1. **Three levels.** *Lobby* = the connected players who are not in a game (server level); *party* =
   a group of players who stay together between games, with a leader; *room* = one game. Every player
   is always in a party (a party of 1 on connection); whether a party of 1 may start a game is a game
   rule. Parties are complete in Part 1 (invite, answer, leave, leader, enter a room together, come
   back together). Automatic matchmaking is Part 2.
2. **Game-facing API.** `Lobby` (server, `LobbyContext::lobby()`), `RoomHandle` (`Context::room()`),
   `ClientLobby` (client, `Context::lobby()`), `RoomNetwork` (`Context::network()` in a room), and the
   events of reference 14. Client requests reach the game as lobby events (`InviteRequested`,
   `InviteAnswered`, `LeavePartyRequested`, `RoomListRequested`); the game answers with `Lobby`
   methods. Starting a game is a game message (for example `PlayRequest`), not an engine method.
3. **Three contexts** (amends ADR 0022): `ServiceContext` (shared services: log, read-only CVars,
   assets, metrics, role), `Context` (one per room on the server, built by the engine when the room is
   created; the game's world on the client) and `LobbyContext` (one, for lobby callbacks on the main
   thread). World access in lobby code does not compile. Game code never creates or stores a context.
4. **Per room:** world, Commands, SceneManager, EventBus, Time, NetworkIdMap, system instances.
   **Shared:** TypeRegistry (frozen after boot), Logger, CVars (written by the lobby only),
   AssetManager (locked on the server), Metrics (series per room).
5. **End of a game:** the room calls `reportFinished(result)`; the lobby receives `RoomFinished`; the
   game decides and calls `closeRoom` (or keeps the room for a rematch). A room cannot stop the server.
   A system that throws in a room stops that room (`RoomFailed`); other rooms continue; the game decides.
6. **Network per room:** `RoomNetwork` sees only the room's players; game messages are routed to the
   player's room or to the lobby; replication sends only the room's entities and resets a player's
   state when they change room. The client changes scene on `EnteredRoom` / `ReturnedToLobby` itself.
7. **Game state of a room** lives in its world, with `Registry::singleton<T>()` for one-per-world data.
8. **Game module hooks:** `onStart(Context&)` stays for the client; the server calls
   `onLobbyStart(LobbyContext&)` once and `onRoomStart(Context&)` for every room; subscriptions that
   belong to a room are kept with `ctx.room().keep(subscription)` and die with the room;
   `onShutdown` receives a `ServiceContext&`.
9. **Systems are instantiated once per room** (option B): a functor with members gets its own copy in
   every room (`Scheduler::add` copies, `addFactory` builds).

## Alternatives considered

- **One `Context` with a world that asserts in the lobby:** errors at run time instead of compile time.
- **Engine applying party rules by default:** contradicts "mechanics in the engine, rules in the game".
- **Forbidding state in systems (option A):** safe only by discipline; B makes it safe by construction.
- **A solo player outside any party:** two code paths for entering a room instead of one.

## Consequences

- New block `room/` (reference 14) and server tasks SRV-08 to SRV-15.
- `Engine` on the server runs a lobby loop on the main thread and rooms on the worker pool; the client
  is unchanged except for `ClientLobby` and the new events.
- Game code written for one game keeps working: systems still read `ctx.world()`, now the room's.
