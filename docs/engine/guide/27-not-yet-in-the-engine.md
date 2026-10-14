# 27 · What is not in the engine yet

So nobody looks for a feature that does not exist. Each line says where it will be decided.

| Feature | Status | Where it will come from |
|---|---|---|
| **UI widgets** (buttons, panels, focus, menu navigation with keyboard and gamepad) | not designed | a new engine block, to be designed and recorded as an ADR |
| Physical response (pushing apart, bouncing, gravity) | not planned in Part 1 | Part 2, Track 1 (physics subsystem); today games block with their own systems (page 18) |
| Particles | not designed | Part 2, rendering subsystem |
| Asynchronous loading and loading screens | API ready, synchronous today | Part 2 (same `SceneManager` API) |
| Editor (hierarchy, inspector, scene save) | planned | Part 2, Track 1 (task ENG-59) |
| raylib backend and 3D | planned | Part 2 (task ENG-58) |
| Hot reload of the game module (C++) | not planned | Track 1 option; scripts and assets already hot-reload |
| Client-side prediction, reconciliation, delta snapshots, compression | written by the game team with engine means (ADR 0041); the means are not designed yet | Network part (Track 2) |
| Protocol bytes (header, message ids, reliability scheme, byte order) | not designed | Network part, protocol RFC |
| Admission by the game (accept or refuse a player with your reason) | decided (ADR 0042), API not designed | Network part, step 4 |
| Rooms and lobby (several games on one server) | designed (ADR 0038, 0039), not implemented | tasks SRV-08 to SRV-15, guide 28 |
| Dedicated room hosting (one process per room, client redirection) | planned | Part 2, Track 1 (`ProcessRoomHost`, ADR 0038) |
| Parallel systems inside one game (job system) | not designed | Part 2, Track 1; the scheduler is single-threaded per room (ADR 0028) |
| Interest management (send each player only what concerns them) | not designed | Network part / Track 2; today every `Replicated` component is sent |
| World streaming (worlds larger than memory) | not planned | Part 2, Track 1 if a game needs it |
| Solo play, matchmaking | not designed | Server part |
| Random number service | not planned | games own a seeded generator (page 26) |
| Localization (several languages) | not planned | — |

If you need one of these before it exists, raise it with the team rather than building it inside a
game module: engine features belong in the engine.
