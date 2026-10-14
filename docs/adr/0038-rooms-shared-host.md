# ADR 0038: Several games on one server: rooms, hosted in-process first

- Status: Accepted (API in [ADR 0039](0039-room-api.md))
- Date: 2026-10-14
- Deciders: the R-Type team
- Discussion id: SRV-5 (Server part, round 2)

## Context

Several groups of players must be able to play at the same time on the same server without sharing a
game. The engine had one active scene per process (ADR 0034). The engine is also meant to serve
bigger games later, where industry engines run one process per match (Unreal, Source) while
room-based servers host many matches per process (Photon, Nakama, Colyseus).

## Decision

1. **The room is the unit of play.** A room is an isolated simulation: its own world, systems run,
   events, tick and players. Game systems are written once and run for every room; `ctx` gives the
   room's world.
2. **Mechanics in the engine, rules in the game.** The engine creates, lists, joins and leaves rooms,
   knows which player is in which room and sends to a room. The game decides room size, when a game
   starts, which level, and what happens when a player leaves.
3. **Where a room runs is a hosting choice behind `IRoomHost`.**
   - Part 1: `ThreadPoolRoomHost` ("shared"): one process, one port, a lobby, and a fixed pool of
     worker threads (about one per core) that advances all rooms in parallel; a room is never run by
     two threads at once, so game code needs no locks. This is also the server's multithreading.
   - Later (Part 2, Track 1 demonstration): `ProcessRoomHost` ("dedicated"): one process per room,
     clients redirected to it. Switching is a configuration change, not a game change.
4. **Four rules keep both hosts possible:** (1) the state of a game lives in its room, never in the
   game module object or in globals; (2) the game creates rooms through the engine and receives a
   `RoomId`; (3) the lobby and the rooms communicate only through messages, never through pointers;
   (4) the protocol reserves a redirection message ("your room is at this address").
5. `server.yaml` gets `rooms.maxRooms` (default 8) and `rooms.workerThreads` (0 = number of cores).
   `net.maxClients` keeps its meaning (connections to the whole server, lobby included) and its
   default goes from 4 to 32 (8 rooms × 4 players), so the default configuration can fill every room.
   8 is a default to adjust after measurement, not a measured limit.

## Alternatives considered

- **All lobby logic in the game:** every game rewrites the same network mechanics.
- **Lobby fully in the engine:** the engine would impose game rules (ready checks, start conditions).
- **Dedicated (process per room) in Part 1:** about three times the server tasks (process spawning on
  two systems, lobby-room channel, client redirection, supervision, port ranges), harder debugging,
  multi-process rather than multithreaded, a port range to open on the school network; little gain
  for light 2D games of four players.
- **One thread per room:** hundreds of threads for hundreds of rooms; a fixed pool scales with cores.
- **One game per server:** does not let independent groups play at once on one server.

## Consequences

- The engine gains a room layer: rooms, a lobby, a host, and per-room world, events, commands and
  replication. Classes, methods, files and tasks come with the next round.
- Game state that used to sit in module members (for example a set of ready players) must be stored
  in the room.
- A crash in one room's system closes that room only; a crash of the engine itself still stops every
  room in shared mode (the dedicated host is the answer for games that need more isolation).
- Big-game needs inside one match (parallel systems, interest management, world streaming) are not
  solved by rooms; they are listed in guide page 27.
