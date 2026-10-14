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
| Client-side prediction, reconciliation | written by the game team with the engine's data (`lastInputProcessed`, RFC §9.4; ADR 0041) | the game |
| Delta snapshots against a baseline, compression | not in protocol v1 (v1 sends only changed components until acknowledged, RFC §9.3) | Part 2, Track 2, protocol version 2 |
| Protocol bytes (header, message ids, reliability scheme) | specified: RTEP v1.0, [`docs/network/rfc.md`](../../network/rfc.md) | tasks ENG-38, SRV-01 to SRV-07, SRV-16 to SRV-20 |
| Admission by the game (accept or refuse a player with your reason) | designed (ADR 0042, 0043), not implemented | task SRV-02 |
| Rooms and lobby (several games on one server) | designed (ADR 0038, 0039), not implemented | tasks SRV-08 to SRV-15, guide 28 |
| Dedicated room hosting (one process per room, client redirection) | planned; `RoomRedirect` id reserved in the RFC | Part 2, Track 1 (`ProcessRoomHost`, ADR 0038) |
| Parallel systems inside one game (job system) | not designed | Part 2, Track 1; the scheduler is single-threaded per room (ADR 0028) |
| Interest management (send each player only what concerns them) | designed: `RoomNetwork::setRelevance` (ADR 0043), not implemented | task SRV-17 |
| World streaming (worlds larger than memory) | not planned | Part 2, Track 1 if a game needs it |
| Solo play | decided: a player alone is a party of 1; the game decides whether solo is allowed (ADR 0039) | guide 28 |
| Matchmaking (quick join, private codes) | not designed | Part 2 |
| Random number service | not planned | games own a seeded generator (page 26) |
| Localization (several languages) | not planned | — |

If you need one of these before it exists, raise it with the team rather than building it inside a
game module: engine features belong in the engine.
