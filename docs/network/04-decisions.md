# Network part · Step 4: design choices

The 37 questions of step 2, decided layer by layer from the bottom. D1 was chosen by the team; from D2
on, the team delegated the choices to the architect's recommendations (2026-10-14). Each decision
names its options, what real protocols do and why. All of them are written into the
[RFC](rfc.md) (step 5); the game-facing changes are recorded in [ADR 0043](../adr/0043-protocol-v1-game-api.md).

| Round | Layer | Decisions | Questions answered |
|---|---|---|---|
| 1 | Frame | D1–D8 | 1, 21, 25, id ranges, byte order |
| 2 | Session | D9–D18 | 1–11, 32–37 |
| 3 | Reliability | D19–D21 | 20, 22, 24 |
| 4 | Replication | D22–D26 | 3, 23, 26, 27 |
| 5 | Lobby, party, rooms | D27–D36 | 12–19, 28–31 |

## Round 1 · Frame

### D1 · Byte order: big-endian

Every multi-byte number on the wire, UDP and TCP, engine and game, is **big-endian**. Floats are IEEE
754, sent as their bit pattern (`std::bit_cast`) in the same order. Conversion lives only in
`ByteWriter` / `ByteReader` (ENG-24).
Options: big-endian (IP, TCP, UDP, QUIC, the CMP spec, the subject's example, Corewar) or
little-endian (CPUs, Quake 3, Unreal, Protobuf). Why: dumps read directly, consistent with the subject
and RFC tradition, cost invisible.

### D2 · Recognising our packets: magic and version

Every UDP datagram and the start of each TCP stream carry the magic `0x52 0x54` ("RT") and the
protocol version (`0x01`). A different version is refused at the handshake with `ProtocolMismatch`,
distinct from `FormatMismatch` (game types).
Options: magic + version (subject, CMP), hidden protocol id + CRC32 (netcode.io), nothing (QUIC, which
is encrypted). Why: readable, a Wireshark dissector recognises our packets alone; UDP's checksum
already catches corruption and bounded reads catch the rest.

### D3 · UDP packet header (16 bytes)

`magic u16 · version u8 · kind u8 · sessionKey u32 · sequence u16 · ack u16 · ackBits u32`.
`kind` is `UdpLink` (handshake and relink) or `Data` (everything else). `sessionKey` is a random
value given at acceptance: a packet whose key does not match its sender's session is dropped, which
stops blind spoofing. Sequence and acks serve the reliability layer (D19).
Options: per-message headers only (ENet), a connection id (QUIC). Why: one fixed header, cheap to
validate before reading anything; the key costs 4 bytes and closes the cheapest attack.

### D4 · TCP framing

Each side starts its stream with the 3-byte preamble (magic, version). Then each message is
`length u32 · id u16 · payload`, `length` counting id and payload, at most 65 536 (ADR 0037); a larger
announced length closes the stream.
Why: the stream has no datagram boundaries; a fixed length prefix is the simplest safe cut.

### D5 · Message ids: u16, one space, two ranges

`0x0000` is invalid; `0x0001`–`0x00FF` belong to the engine (session, lobby, rooms, replication);
`0x0100`–`0xFFFF` belong to the game, `0x0100 + index` in `TypeRegistry::messages()` order (already
decided). The same ids are used on TCP and UDP; the transport of an id is fixed by its declaration.
Why: engine and game messages share one mechanism (step 3); ranges keep them from colliding.

### D6 · Several messages per datagram

A `Data` packet carries any number of messages: `id u16 · [messageSeq u16 if reliable] · length u16 ·
payload`. The receiver knows from the id whether `messageSeq` is present (same registry, guaranteed by
the fingerprint). Outgoing messages are packed once per tick (LateUpdate); a datagram never exceeds
1200 bytes.
Options: one message per datagram (simple, 16 bytes of header each). Why: inputs, acks, resends and
game messages share headers; standard in ENet, Quake, Unreal bunches.

### D7 · No generic fragmentation

A message must fit in one datagram. Snapshots are cut by the replication layer into **self-contained
parts** (each entity entry stands alone), so a lost part loses only its entities for one tick. A
reliable or unreliable message larger than 1024 bytes is refused at `send` with a log line telling to
use `.stream()`.
Options: fragment and reassemble (more state, a lost fragment loses the whole message). Why: Glenn
Fiedler's and Valve's advice for real-time data; big payloads belong on TCP.

### D8 · Field encoding

Fields are written in declaration order, without names or padding: `Bool` u8 (0 or 1, anything else
fails), `Int32` i32, `UInt32` u32, `Int64` i64, `Float` f32 bits, `Double` f64 bits, `String` u16 byte
length + UTF-8 (limit `maxLength` in bytes), `Vec2` 2×f32, `Rect` 4×f32, `Color` 4×u8 (RGBA), `Entity`
u32 `NetworkId` (0 = null), asset handles as their path (`String`), `Enum` its underlying value as
i32 (unknown value fails), `List` u16 count + elements (limit `maxLength`), `Struct` its fields,
`Custom` the author's `binary()` codec. A quantised float (D26) is a u16.

## Round 2 · Session

### D9 · Hello and the order of checks

Hello (client → server, TCP) carries: name, format fingerprint u64, game name (≤ 32, the module name
given to `loadGame`), reconnection token (16 bytes, zero if none), optional game data (one game
message). The server checks in this order and stops at the first failure: preamble and version
(`ProtocolMismatch`), game name (`WrongGame`), fingerprint (`FormatMismatch`), name (`InvalidName`),
capacity (`ServerFull`), then the game's admission (`RefusedByGame` with the game's text, D13).
New reasons: `ProtocolMismatch`, `WrongGame`, `InvalidName`, `RefusedByGame`, `ConnectionLost`.
`LeftServer` and `PlayerLeft` gain a `message` text.
Why: cheapest and most certain checks first; the game is asked only about a client the engine can
decode (ADR 0042); "wrong game" gets a clear message instead of "rebuild both".

### D10 · Challenge and UDP link

Challenge carries a random 64-bit `linkToken`. The client sends `UdpLink(linkToken)` every 100 ms
until Accepted arrives; the handshake must finish within 5 s (`net.handshakeSeconds`), else
`Refused(Timeout)`. The server **never answers** an unauthenticated UDP packet (no amplification);
success is told by Accepted on TCP.

### D11 · Accepted

Carries `playerId u32`, `sessionKey u32`, a new reconnection token (16 bytes), `resumed u8`, and the
server's timing: tick rate, snapshot rate, heartbeat, timeout, grace period. The client adopts them.
Followed by `PartyUpdated` (party of 1, or the old party when resumed) and the online players list
(D27).

### D12 · Flood limits

At most 16 pending handshakes (`net.maxPendingHandshakes`) and 4 per IP address; extra TCP
connections are closed at once and counted. A session sending more than 50 invalid packets in 10 s is
closed with `ProtocolError`. More than 300 packets per second from one session: the excess is dropped
and counted. Unknown endpoints get nothing but `UdpLink` processing.

### D13 · Admission extension point (ADR 0042)

Client: `connectTo(host, port, name, data)` where `data` is any game message (for example
`JoinInfo{version}`). Server, in lobby code: `lobbyCtx.network().admission<T>(fn)`; `fn(const
AdmissionRequest&, const T&)` returns `Admission::accept()` or `Admission::refuse("text")`. Called on
the lobby thread, synchronously, after the engine's checks, and again on reconnection
(`request.reconnecting`). No handler: everyone the engine accepts gets in.
Why: same shape as Unreal `PreLogin` and Unity `ConnectionApproval`, but typed (ADR 0037).

### D14 · Heartbeat

The heartbeat is a `Data` packet with no message, sent only when nothing else was sent for 0.5 s; any
packet keeps the session alive and carries acks. RTT and loss come from packet acks (D19). On TCP, a
`KeepAlive` message is sent after 10 s of silence so NAT boxes keep the stream.
Why: in game, inputs and snapshots already flow; the UDP path is the one to prove alive (TCP can
stay up while UDP is cut).

### D15 · Leaving

`Bye` (TCP) → `Quit`. TCP closed without Bye → `ConnectionLost`. No packet for 5 s → `Timeout`.
`Kicked`, `ServerShutdown` and `Refused` carry a text (≤ 128 bytes) chosen by the game or the engine.
`ConnectionLost` and `Timeout` start the grace period (D16); the others end the session at once.

### D16 · Reconnection

Grace period 30 s by default (`net.reconnectGraceSeconds`, 0 disables). During it the player is
"disconnected", not removed: the server publishes `PlayerDisconnected{player, reason}` (lobby, and the
room if any), `PartyUpdated` marks the member, and the game applies its policy (wait, pause, AI, kick).
Back in time with the token: same `PlayerId`, `PlayerReconnected{player}`, `Accepted(resumed)`, party,
and the room through `EnteredRoom` (D31). Grace over: `PlayerLeft{reason}`. Client side, the engine
reconnects by itself every 2 s (`net.autoReconnect`, default on) and publishes
`ConnectionInterrupted{reason}`, then `Reconnected{}`, or `LeftServer{reason}` when it gives up.
Why: mechanism in the engine, policy in the game (Server round 4); the subject asks to rejoin with a
token.

### D17 · UDP address change

When nothing has arrived on UDP for 1 s while TCP is open, the client sends `UdpLink(linkToken)`
again; the server, which has the token from this session's TCP handshake, adopts the new address.
Why: NAT rebinding is common on Wi-Fi; the server cannot detect it (packets from an unknown address
are dropped), the client can.

### D18 · Tokens

Link tokens (64 bits), session keys (32 bits) and reconnection tokens (128 bits) come from the
operating system's secure random source. A reconnection token is single use (each Accepted gives a new
one), bound to its `PlayerId`, and dies with the grace period. It lives in memory only: a client that
crashed and restarts is a new player.
Why: tokens travel in clear (encryption is out of scope): unpredictable, single use and short-lived is
the minimum.

## Round 3 · Reliability

### D19 · Packet acknowledgements

Every `Data` packet has a sequence number (u16, per direction, wrapping, compared modulo 2¹⁶) and
acknowledges the latest received sequence plus the 32 before it (`ackBits`). Duplicates and packets
more than 32 behind the latest are dropped. If reliable data arrived and nothing is sent within 33 ms,
an ack-only packet goes out. RTT is measured from acks (smoothed), loss from unacknowledged packets.
Why: Glenn Fiedler's scheme ("Reliability and ordering over UDP"), used by many action games: every
packet acknowledges, redundantly, so acks survive loss without their own reliability.

### D20 · Reliable messages

A reliable message has a `messageSeq` (u16, per direction). It is resent in a new packet when the
packet that carried it is not acknowledged after `clamp(1.5 × RTT, 50 ms, 1 s)`. Delivery is **in
order**, one ordered stream per direction; the receiver holds up to 256 messages ahead and drops
duplicates. At most 256 unacknowledged messages in flight; more are queued locally. A message is never
abandoned: only the session's timeout ends it.
Options: ordered per message type (more state, little gain for a few messages per second). Why: one
order is simple to reason about and keeps room transitions after the messages that precede them (D31).

### D21 · Replication: send until acknowledged

Spawns, destroys and component changes are sent in snapshots **until a packet carrying them is
acknowledged** by that client. For each client the server keeps, per entity, whether its spawn was
acknowledged and a hash of each component's last acknowledged bytes; a component goes out when its
bytes differ. A lost snapshot simply means its content goes again in the next one.
Options: spawn and destroy as reliable messages (ordering problems with unreliable updates); full state
every time (64 KB/s worst case). Why: Quake 3 and Tribes' approach; guarantees spawns and destroys
(question 20), uses the snapshot acks for free (question 22), and cuts bandwidth to what changes.

## Round 4 · Replication

### D22 · Snapshot message

`roomSession u16 · tick u32 · lastInputProcessed u32 · count u16 · entries`. An entry is
`networkId u32 · op u8` (`0` update, `1` spawn, `2` destroy); a spawn adds `prefabKey u32`; spawns and
updates add `componentCount u8` and, per component, `componentIndex u16 · length u16 · fields`
(`componentIndex` = position among `Replicated` components in registration order). Several snapshot
messages may carry one tick (D7); the client applies an entry only if its tick is newer than the last
one applied to that entity. `NetworkId`s are never reused (ADR 0033), so the client remembers
destroyed ids, including a destroy for an id it never saw, and ignores any later entry for them: a
spawn overtaken by its destroy can never leave a ghost.

### D23 · Input message and redundancy

`roomSession u16 · newestSeq u32 · count u8 (1–4) · masks u64 × count`, newest first. Each input is
sent 3 times by default (`net.inputRedundancy`). The server keeps up to 4 pending inputs per player,
applies one per tick in order, drops duplicates and old ones, and when the buffer overflows drops the
oldest. `lastInputProcessed` in that player's snapshots tells the client which input the state
includes (prediction, ADR 0041).

### D24 · Time

`EnteredRoom` carries the room's current tick and tick rate; snapshots carry their tick. The client
estimates the server tick from them and the RTT, and renders other entities `net.interpolationDelay`
behind (default 0.1 s, two snapshots). Inputs are numbered by the client, not by server tick.

### D25 · Entity named before its spawn

Inside a packet, replication is applied before messages, so a message travelling with the spawn finds
its entity. `Entity::null()` stays the rule when it does not (ADR 0037).

### D26 · Settings exposed to the game (ADR 0041)

| Setting | Default | What it tunes |
|---|---|---|
| `net.snapshotRate` | 20 | snapshots per second |
| `net.sendBudgetBytesPerSecond` | 65 536 | per player; snapshots stop filling at the budget |
| `NetPriority{weight}` component | 1 | who goes first when over budget (accumulated, so nobody starves) |
| `RoomNetwork::setRelevance(fn)` | everything in the room | which entities a player receives |
| `.quantize(min, max)` on float or `Vec2` fields | off | the field travels as u16 instead of f32 |
| `net.interpolationDelay` | 0.1 s | how far behind other entities are drawn |
| `engine.net.interpolate` system | on | disable to write your own |
| `net.inputRedundancy` | 3 | copies of each input |
| `net.reconnectGraceSeconds`, `net.autoReconnect` | 30, on | reconnection |

With a 4 KB/s budget, quantised positions and priorities, a game can stay playable on the subject's
5 KB/s link (scope, step 1).

## Round 5 · Lobby, party, rooms

### D27 · Online players

The engine keeps the list of connected players (id, name) and sends it to lobby clients:
`OnlinePlayers` after Accepted, then `PlayerOnline` / `PlayerOffline`. Client:
`ctx.lobby().onlinePlayers()`, event `OnlinePlayersChanged`. A game can turn it off
(`net.shareOnlinePlayers`, default on).

### D28 · Names in parties

`PartyUpdated` carries each member's id, name and connected flag (D16).

### D29 · Refused requests

`Lobby::refuse(player, request, reason)` sends `RequestRefused{request, reason}`; the client publishes
`RequestRefused`. A game that ignores a request stays silent, by its choice.

### D30 · Invitations during a game

Delivered (TCP); the client game decides whether to show them.

### D31 · Room transitions on the reliable UDP stream

`EnteredRoom`, `RoomReady`, `ReturnedToLobby` and `RoomInterrupted` travel as reliable UDP messages,
in the same ordered stream as the game's reliable messages. Each entry into a room gets a new
`roomSession` (u16, per player). The server sends snapshots to a player only after `RoomReady{roomSession}`
(scene loaded); snapshots and inputs of another `roomSession` are dropped.
Why: one ordered stream removes the TCP/UDP race at entry (16) and at exit (28); `RoomReady` handles
loading (17, slide 20's "loading screen"); `roomSession` drops late packets of the previous room (18).

### D32 · EnteredRoom

`roomId u32 · roomSession u16 · name · level · maxPlayers u8 · listed u8 · roomTick u32 · tickRate u16`.

### D33 · Failed room

As soon as a room fails, the engine sends `RoomInterrupted{roomId}` to its players (client event
`RoomInterrupted`), then the game decides (usually `closeRoom`). Heartbeats continue, so nobody times
out.

### D34 · Reasons for leaving a room

`closeRoom(room, reason = "")` and `moveToLobby(party, reason = "")`; the reason reaches
`ReturnedToLobby{room, reason}`.

### D35 · Results

Results are a game message. Sent `reliable()` before `closeRoom`, they arrive before `ReturnedToLobby`
(D20, D31). The engine does not forward `RoomFinished` to clients.

### D36 · Reserved for Part 2

`RoomRedirect{address, port, token}` (id reserved) for dedicated room processes, and the lobby ↔ room
process channel (ready, heartbeat, finished, crash), inventoried with Part 2.

## Questions → decisions

| Question | Decision | Question | Decision |
|---|---|---|---|
| 1 protocol version | D2, D9 | 20 spawns guaranteed | D21 |
| 2 wrong game | D9 | 21 snapshot over 1200 bytes | D7, D22 |
| 3 Accepted, tick sync | D11, D24 | 22 snapshot acks | D19, D21 |
| 4 handshake delay, flood | D10, D12 | 23 input redundancy | D23 |
| 5 landing in the lobby | D11 | 24 reliable order | D20 |
| 6 admission | D13 | 25 aggregation | D6 |
| 7 heartbeat channel | D14 | 26 entity before spawn | D25 |
| 8 heartbeat when idle | D14 | 27 settings | D26 |
| 9 crash reason | D15 | 28 results vs return | D31, D35 |
| 10 TCP closed without Bye | D15, D16 | 29 failed room | D33 |
| 11 kick reason | D15 | 30 reason of return | D34 |
| 12 online players | D27 | 31 results forwarded | D35 |
| 13 member names | D28 | 32 grace length | D16 |
| 14 refused requests | D29 | 33 reconnection events | D16 |
| 15 invitations in game | D30 | 34 UDP address change | D17 |
| 16 TCP/UDP at entry | D31 | 35 token after crash | D18 |
| 17 loading time | D31 | 36 admission on return | D13 |
| 18 late packets | D31 | 37 token theft | D18 |
| 19 EnteredRoom content | D32 | | |
