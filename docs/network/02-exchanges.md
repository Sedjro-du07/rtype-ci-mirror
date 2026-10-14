# Network part · Step 2: inventory of exchanges

Every exchange, as a sequence diagram, before any byte. For each: direction, when, size, whether it
must arrive, whether order matters, transport. Groups follow a player's life:

| Group | Exchanges | Status |
|---|---|---|
| 1 | Connection: handshake, refusals, TCP ↔ UDP link | validated |
| 2 | Presence: heartbeat, timeout, voluntary leave, kick, shutdown | validated |
| 3 | Lobby and party: invitations, room list | validated |
| 4 | Entering and leaving a room | validated |
| 5 | In game: inputs, snapshots, spawns and destructions, game messages | validated |
| 6 | End of game, back to the lobby, failed room | validated |
| 7 | Reconnection | validated |

## Group 1 · Connection

Two handshakes sit on top of each other. TCP's (SYN, SYN-ACK, ACK) is done by the operating system on
`connect`; the RFC only mentions it. **Ours** is Hello → Challenge → UdpLink → Accepted (or Refused):
it is the one the RFC defines and the state machine of task SRV-02.

### Normal connection

```
Client                                         Server
  │── TCP connect (SYN / SYN-ACK / ACK, by the OS) ─▶│
  │── Hello (name, fingerprint, game data) ── TCP ──▶│  engine: protocol version? fingerprint? room left?
  │                                                  │  game: admission (ADR 0042)
  │◀──────────────────────── Challenge (token) TCP ──│  connection kept "pending"
  │── UdpLink (token) ──────────────────────── UDP ─▶│  repeated until answered
  │                                                  │  right token → the player's UDP address
  │◀──────────────────────── Accepted (PlayerId) TCP │
  JoinedServer{me}                         PlayerJoined{player}
```

The token prevents spoofing: only who received it on TCP can declare its UDP address. No third
message is needed: TCP delivers Accepted, and Accepted tells the client its UdpLink arrived.

### Refused

```
Client                                         Server
  │── Hello ──────────────────────────────── TCP ──▶│  engine check fails, or the game refuses
  │◀──────────────────────────── Refused (reason) ──│  then closes
  LeftServer{reason} + a log line saying what to do
```

### The UDP link never arrives (firewall, losses)

```
Client                                         Server
  │── UdpLink ── UDP ──✕                             │
  │── UdpLink ── UDP ──✕      … handshake delay …    │
  │◀─────────────────────────── Refused (Timeout) ──│  frees the place
  LeftServer{Timeout} + log "UDP blocked?"
```

### The client disappears during the handshake

It quits, or its TCP connection drops: the server frees the pending place. No event reaches the game:
the player was never accepted.

### Inventory

| Exchange | Direction | When | Size | Must arrive | Order | Transport |
|---|---|---|---|---|---|---|
| Hello | C → S | once | small (name ≤ 16, fingerprint, game data) | yes | yes | TCP |
| Challenge | S → C | once | tiny (token) | yes | yes | TCP |
| UdpLink | C → S | repeated until answered | tiny | no: repeated | no | UDP |
| Accepted | S → C | once | small | yes | yes | TCP |
| Refused | S → C | once, then close | tiny + reason | yes | yes | TCP |

### Questions for step 4

1. Protocol version separate from the format fingerprint, checked before reading the rest of the Hello.
2. Wrong game (a Pong client on an R-Type server): a clear reason, not "rebuild both".
3. Content of Accepted: `PlayerId`, and what synchronisation needs (tick rate, current server tick,
   snapshot rate; ADR 0041).
4. Handshake delay and number of pending connections (flood).
5. Where an accepted player lands: the lobby, in a party of 1 (ADR 0039); starts group 3.
6. Admission extension point (ADR 0042): game data in the Hello, admission request, game's reason.

## Group 2 · Presence

How each side knows the other is still there, and how they part. Already decided (ADR 0033, 0037):
heartbeat every 0.5 s, timeout after 5 s without any packet, reasons `Quit`, `Timeout`, `Kicked`,
`ServerShutdown`, `ProtocolError`.

### Normal state

```
Client                                         Server
  │── Heartbeat ─────────────────────────── UDP ─▶│  every 0.5 s
  │◀──────────────────────────── Heartbeat ─ UDP ─│  every 0.5 s
  (any packet received counts as "alive")
```

The heartbeat also measures the ping (`ping(p)`, console `net`).

### Voluntary leave

```
Client                                         Server
  │  disconnect()                                 │
  │── Bye ─────────────────────────────────── TCP ─▶│
  │   closes                                      │  PlayerLeft{player, Quit}
  LeftServer{Quit}
```

### The client crashes

```
Client ✕ (process killed)                      Server
  │  the OS closes its sockets ── TCP closed ────▶│  PlayerLeft{player, ?}
```

The server sees it at once, without waiting 5 s.

### The machine or the network disappears

```
Client ✕                                       Server
  │            ✕ nothing arrives any more ✕       │  … 5 s without a packet …
  │                                               │  PlayerLeft{player, Timeout}
  (client side: LeftServer{Timeout} after 5 s without news from the server)
```

TCP reports nothing here; only our timeout detects it.

### Kick

```
Client                                         Server
  │◀──────────────────────── Kicked (reason) ─ TCP ─│  kick(p, reason)
  │                                         closes │  PlayerLeft{player, Kicked}
  LeftServer{Kicked} + the reason shown
```

### Server shutdown

```
Client(s)                                      Server
  │◀────────────────────────── ServerShutdown TCP ─│  stopServer(), sent to all
  │                                         closes │
  LeftServer{ServerShutdown}
```

### Garbage in a loop

The server disconnects the client with `ProtocolError`. What "in a loop" means (how many invalid
packets, over how long) is decided with the flood threat in step 4.

### Inventory

| Exchange | Direction | When | Size | Must arrive | Order | Transport |
|---|---|---|---|---|---|---|
| Heartbeat | C ↔ S | every 0.5 s | tiny | no: the next replaces it | no | UDP |
| Bye | C → S | once | tiny | yes | yes | TCP |
| Kicked | S → C | once, then close | tiny + reason | yes | yes | TCP |
| ServerShutdown | S → all | once, then close | tiny | yes | yes | TCP |
| TCP close | both | crash, end | (OS) | — | — | TCP |

### Questions for step 4

7. Heartbeat on UDP, TCP or both. Proposed UDP: it proves the game's path works; TCP can stay open
   while UDP is cut.
8. Heartbeat only when no other traffic: in game, inputs and snapshots already flow.
9. Reason for a crash (TCP closed without Bye): neither `Quit` nor `Timeout`; a "connection lost"?
10. TCP closed without Bye: immediate leave, or start of a grace period for reconnection (group 7).
11. Kick reason: free text from the game (as for admission, ADR 0042) or a fixed list.

## Group 3 · Lobby and party

Already decided (ADR 0039, reference 14): the client sends requests, the server turns them into
events, the game decides by calling the `Lobby`. The engine carries and applies.

### Arriving in the lobby (after group 1)

```
Client                                         Server
  │◀────────────────────────────── Accepted ── TCP ─│
  │◀───────────────── PartyUpdated (party of 1) ────│  the player leads their party of 1
```

### Inviting a friend

```
Client A (leader)          Server                        Client B
  │── InviteRequest(B) ──▶│ InviteRequested{A, B}          │
  │                       │ game: A is leader? party full? │
  │                       │ lobby.sendInvite(party, B)     │
  │                       │── InviteReceived(A, party) ───▶│  popup
  │                       │◀── InviteAnswer(party, yes) ───│
  │                       │ InviteAnswered → game: addToParty
  │◀── PartyUpdated ──────│── PartyUpdated ───────────────▶│  to every member
```

### Leaving one's party

```
Client                                         Server
  │── LeaveParty ──────────────────────────── TCP ─▶│  LeavePartyRequested → game: removeFromParty
  │◀───────────────── PartyUpdated (party of 1) ────│  former members get PartyUpdated too
```

### A member disconnects

The server removes the player; if they led, another member leads (a party always has a leader). The
remaining members receive `PartyUpdated`.

### Room list

```
Client                                         Server
  │── RoomListRequest ─────────────────────── TCP ─▶│  RoomListRequested → the game picks which rooms
  │◀───────────────────── RoomList (RoomInfo…) TCP ─│  lobby.sendRoomList(player, rooms)
  RoomListReceived{rooms}
```

The game decides which rooms to show (for example hides private ones).

### Game messages in the lobby

For example party chat: `lobby.sendToParty(party, msg)`; typed messages of the game, with the
transport the game chose.

### Inventory

| Exchange | Direction | When | Size | Must arrive | Order | Transport |
|---|---|---|---|---|---|---|
| InviteRequest | C → S | player action | tiny | yes | yes | TCP |
| InviteReceived | S → C | if the game accepts | small | yes | yes | TCP |
| InviteAnswer | C → S | player action | tiny | yes | yes | TCP |
| LeaveParty | C → S | player action | tiny | yes | yes | TCP |
| PartyUpdated | S → members | on every change | small | yes | yes (last state wins) | TCP |
| RoomListRequest | C → S | player action | tiny | yes | yes | TCP |
| RoomList | S → C | answer | medium (≤ `maxRooms` entries) | yes | yes | TCP |
| Game messages | both | the game's | the game's | the game's | the game's | chosen by the game |

All on TCP: out of game, allowed by the subject for the lobby.

### Questions for step 4

12. Online players list: to invite, a client needs ids and names; `ClientLobby` has nothing for it.
    Engine mechanism, or the game's messages?
13. Names of party members: `PartyUpdated` carries `PlayerId`s; clients need names to show.
14. Request refused by the game: today the requester receives nothing; an answer "refused (reason)",
    as for admission?
15. Invitation received during a game: the player stays reachable over TCP; not simulation traffic,
    so the subject allows it. Acceptable?

## Group 4 · Entering and leaving a room

Already decided (ADR 0039, guide 28): starting a game is a rule of the game, so a message of the game;
the game calls `createRoom` then `moveParty`; clients receive `EnteredRoom` and change scene
themselves; in the room the game receives `PlayerEnteredRoom`, and replication starts from zero for
that player.

### Starting a game

```
Client (leader)              Server: lobby              Server: room 7
  │── PlayRequest ─── TCP ─▶│ game: leader? solo allowed?
  │                         │ createRoom(…) → room 7 ───────▶│ loads the level, onRoomStart
  │                         │ moveParty(party, 7) ──────────▶│
  │◀── EnteredRoom(7, settings) ─ TCP ─ (every member)       │
  │  the client changes scene                                │
  │                                                          │ PlayerEnteredRoom{p} → the game spawns the ship
  │◀═══════════════════ snapshots (spawns, positions) ═ UDP ═│
  │═══════════════════════════ inputs ════════════════ UDP ═▶│
```

### Joining an existing room (from the list)

Same as starting, with another message of the game ("join room 7"); the game checks places, then
`moveParty`. No new engine mechanism.

### Room cannot be created

`maxRooms` reached: `createRoom` fails; the game tells its player with its own message. The engine
answers nothing to the client.

### Leaving the current game

```
Client                       Server: lobby              Server: room 7
  │── (game message) ─ TCP ▶│ game: moveToLobby(party)
  │                         │ ──────────────────────────────▶│ PlayerLeftRoom{p} → the game destroys the ship
  │◀── ReturnedToLobby(7, reason) ─ TCP                      │
  │  the client goes back to its lobby scene                 │
```

### Disconnection during a game

Group 2 applies, then the room receives `PlayerLeftRoom{p, Timeout}` (or another reason).
Reconnection: group 7.

### Room in another process (Part 2)

`EnteredRoom` will carry "your room is at this address" and a token; the client runs a group 1
handshake with the room process. The message is reserved (ADR 0038); its content is decided later.

### Inventory

| Exchange | Direction | When | Size | Must arrive | Order | Transport |
|---|---|---|---|---|---|---|
| Game request (play, join, leave) | C → S | player action | tiny | yes | yes | TCP (the game's choice) |
| EnteredRoom | S → members | once | small (room + settings) | yes | yes | TCP |
| ReturnedToLobby | S → members | once | small | yes | yes | TCP |
| Snapshots, inputs | — | — | — | — | — | group 5 |

### Questions for step 4

16. Two channels that do not wait for each other: `EnteredRoom` goes on TCP, the first snapshot on
    UDP and may arrive first. A rule is needed (drop, hold, room number in snapshots…).
17. Loading time: a "ready" message from the client before snapshots, or a first full snapshot sent
    until acknowledged? (Slide 20: the room streams the spawns, that is the clients' loading screen.)
18. Late packets of the previous room after `ReturnedToLobby`: recognise and drop them.
19. Content of `EnteredRoom`: room id, settings, the room's starting tick (with question 3).

## Group 5 · In game

The heart of the protocol and the only traffic the subject forces on UDP. Already decided:
replication (`NetworkId`, `NetPrefab`, `Replicated`, 20 snapshots/s, only the room's entities; ADR
0033, 0039); inputs as an action mask each tick into `RemoteInput` (ADR 0030, 0033); game messages
`unreliable`, `reliable` (UDP + acks) or `stream` (TCP) (ADR 0037); an `Entity` in a message becomes
`Entity::null()` when unknown (ADR 0037); prediction fields in v1: input number and last input
processed (ADR 0041).

### Normal loop

```
Client                                            Server: room
  │══ Input (#41, mask) ══════════════════ UDP ═▶│  60 per second → RemoteInput
  │══ Input (#42, mask) ══════════════════ UDP ═▶│
  │◀═ Snapshot (tick 1200, last input processed 42,
  │            entities: spawn / update / destroy) ═ UDP ═│  20 per second
  │   interpolation (engine default) or the game's own (ADR 0041)
```

### An entity appears, then disappears

```
  │◀═ Snapshot: spawn #57 (NetPrefab "bydos", position) ═│  the client instantiates the same prefab
  │◀═ Snapshot: #57 position …                          ═│
  │◀═ Snapshot: destroy #57                             ═│  the client destroys it
```

If the snapshot carrying the spawn or the destroy is lost, the client never sees the enemy or keeps it
forever: question 20.

### Reliable game message

```
Server                                            Client
  │── WaveCleared{3} (reliable) ══════════ UDP ═▶  ✕ lost
  │   no ack after a delay → resent               │
  │── WaveCleared{3} ═════════════════════ UDP ═▶│  arrived
  │◀══════════════════════════════ ack ═══ UDP ══│
```

The same client → server (for example `Ready`).

### Unreliable game message

Sent once; if lost, the next one replaces it.

### Stream game message during a game

Text chat (`.stream()`) on TCP; allowed by the subject, it is not simulation traffic.

### Loss, delay, duplicates

Lost input: the server keeps the last known state. Lost snapshot: the next one is enough (full state).
Late or duplicated packet: recognised and dropped, which needs sequence numbers (subject, slide 3).

### Inventory

| Exchange | Direction | When | Size | Must arrive | Order | Transport |
|---|---|---|---|---|---|---|
| Input | C → S | 60/s | ~24 bytes | no (redundancy possible) | newest wins | UDP |
| Snapshot | S → C | 20/s | up to ~3 KB worst case | no: the next replaces it | newest wins | UDP |
| Spawn / destroy | S → C | inside snapshots | small per entity | yes | yes (spawn before updates) | UDP |
| Reliable game message | both | the game's | the game's | yes | to decide | UDP + acks |
| Ack of reliable messages | both | on each receipt | tiny | no (repeated) | no | UDP |
| Unreliable game message | both | the game's | the game's | no | no | UDP |
| Stream game message | both | the game's | the game's | yes | yes | TCP |

### Questions for step 4

20. Guaranteeing spawns and destroys: reliable messages, or repeated in every snapshot until the
    client acknowledges?
21. A snapshot over 1200 bytes: split over datagrams, spread over ticks, or a limit?
22. Does the client acknowledge snapshots? Needed for deltas later (an engine mechanism a game uses or
    not through its settings, ADR 0041).
23. Input redundancy: how many past inputs per packet; what the server does with an input too old or
    from the future.
24. Order of reliable messages: guaranteed, or per message type only?
25. Several messages in one datagram (aggregation) to save headers?
26. A message naming an entity before its spawn: keep `Entity::null()`, or hold the message until the
    spawn?
27. Settings exposed to the game (ADR 0041): rate per entity, field precision, who receives what,
    priorities; list them.

## Group 6 · End of game, back to the lobby, failed room

Already decided (ADR 0039, guide 28): the room reports its end (`reportFinished`), the lobby receives
`RoomFinished`, the game decides (usually `closeRoom`), clients receive `ReturnedToLobby`. A system that
throws stops its room, the lobby receives `RoomFailed`, other rooms continue, the game decides. In
Part 1 room and lobby share a process: they talk through an in-memory mailbox, not the network; only
exchanges with clients belong to the protocol.

### Normal end

```
Client(s)                    Server: lobby               Server: room 7
  │                         │                 ◀── reportFinished(victory, 12400)
  │                         │ RoomFinished → the game     │
  │◀── (game message: results screen) ────────────────── │  if the game wants
  │                         │ closeRoom(7)                │  the room stops
  │◀── ReturnedToLobby(7, reason) ── TCP                  │
  │  the client goes back to its lobby scene (parties stay together)
```

### Failed room

```
Client(s)                    Server: lobby               Server: room 7
  │                         │                 ◀── exception in a system
  │   (no more snapshots)   │ RoomFailed{7, error}        │  room stopped
  │                         │ game: closeRoom(7)          │
  │◀── ReturnedToLobby(7, reason) ── TCP                  │
  │  message "the game stopped"
```

The subject requires it (track2_3): a crash never takes the server down, and players are told.

### Server shutdown during games

Group 2: `ServerShutdown` to everyone.

### Room in another process (Part 2)

Lobby and room then talk over the network: ready, heartbeat, finished, crash detected (slide track2_3).
A server-to-server channel, reserved for Part 2 and inventoried then.

### Inventory

| Exchange | Direction | When | Size | Must arrive | Order | Transport |
|---|---|---|---|---|---|---|
| Results (game message) | S → members | end of game | small | yes | before the return to the lobby | chosen by the game |
| ReturnedToLobby | S → members | once | small + reason | yes | yes | TCP |
| Lobby ↔ room | — | — | — | — | — | memory (Part 1), network (Part 2) |

### Questions for step 4

28. Results and return out of order: results sent reliable UDP from the room, `ReturnedToLobby` on
    TCP; results may arrive after the scene change (question 16 the other way round).
29. Telling clients as soon as a room fails: between `RoomFailed` and the game's decision clients
    receive nothing and may believe in a timeout; an engine "room paused" signal?
30. Who gives the reason of `ReturnedToLobby`: the event carries text, but `closeRoom(room)` and
    `moveToLobby(party)` take no reason; the game should be able to give it (as for admission, kick).
31. Room results to clients: forwarded by the engine (`RoomFinished` already holds them), or always a
    game message?

## Group 7 · Reconnection

Already decided (Server part, round 4): the mechanism is in the engine (token, grace period, events),
the policy in the game; minimal rule: a party always has a leader. The subject asks for it
(track2_4: disconnect → rejoin with the token).

### Short cut (under the timeout)

Packets resume before 5 s: no event, the game goes on; losses are absorbed by group 5.

### The UDP address changes without a TCP cut (NAT rebinding)

```
Client                                         Server
  │══ Input ═══ UDP (new address) ══════════▶  ✕ unknown address → dropped
  │── UdpLink (token) ═════════════════ UDP ═▶│  the token proves it is them → new address kept
  │══ Input ═══════════════════════════ UDP ═▶│  works again
```

### Long cut, back within the grace period

```
Client                                         Server
  │  ✕ cut (timeout, or TCP closed without Bye)  │  the player becomes "disconnected", not removed
  │                                              │  game: PlayerDisconnected{p} → its policy
  │                                              │  (wait, pause, AI, end…)
  │   … the client connects again …              │
  │── Hello (… + reconnection token) ─── TCP ──▶│  the engine recognises the player
  │◀───────────────── Challenge / UdpLink ──────▶│  (as group 1)
  │◀──────────── Accepted (same PlayerId) ─ TCP ─│  game: PlayerReconnected{p}
  │◀──────────── PartyUpdated, EnteredRoom ─ TCP ─│  back in their party and room
  │◀═════════════ full snapshot ═════════════ UDP │  the world again
```

### Grace period expires

Final leave: `PlayerLeft{Timeout}`; the room receives `PlayerLeftRoom`, the party is updated and gets a
new leader if needed.

### The client crashed and restarts

The token was in memory and is lost, unless saved; without it, a normal connection (group 1) and a new
player.

### The server restarted

Tokens are lost on the server: normal connection.

### Inventory

| Exchange | Direction | When | Size | Must arrive | Order | Transport |
|---|---|---|---|---|---|---|
| Reconnection token | S → C | inside Accepted | small | yes | yes | TCP |
| Hello with token | C → S | on return | small | yes | yes | TCP |
| UdpLink (new address) | C → S | repeated until answered | tiny | no: repeated | no | UDP |
| Accepted (resumed) | S → C | once | small | yes | yes | TCP |
| PartyUpdated, EnteredRoom | S → C | after resuming | small | yes | yes | TCP |
| Full snapshot | S → C | after resuming | up to ~3 KB | yes (at least one) | — | UDP |

### Questions for step 4

32. Grace period length: a setting the game chooses (ADR 0041); default value?
33. Events for the game: `PlayerDisconnected`, `PlayerReconnected` (new names), in the room and the
    lobby; what other players see: an engine message or the game's?
34. UDP address change: how the client knows it must send UdpLink again (server silence, or a TCP
    signal)?
35. Token after a client crash: saved on disk, or a crash means a new player?
36. Admission on reconnection: does the returning player go through the game's admission again
    (ADR 0042)? They may have been banned meanwhile.
37. Token theft: it travels in clear (encryption out of scope); at least unpredictable, single use,
    bound to the player.

## Step 2 complete

Seven groups, 37 questions for step 4. Next: step 3, layers.
