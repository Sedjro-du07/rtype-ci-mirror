# Network part · Step 1: scope

What the protocol must carry, what the subject requires, the budgets it is designed for and the
threats it must survive. Validated on 2026-10-14. Nothing here decides bytes: the protocol only
decides **how this is written in bytes** and **how exchanges follow each other**.

## A. Already decided, carried by the protocol

| Area | Decided | ADR |
|---|---|---|
| Who defines the protocol | the engine, for every game; a game only defines the content of its messages | 0037, 0040 |
| Transport | `INetwork` plugin on an I/O thread; UDP and TCP from Part 1 | 0033, 0037 |
| Size | datagram ≤ 1200 bytes (larger: dropped, never allocated); TCP message ≤ 64 KB | 0033, 0037 |
| Input robustness | bounded reads (`ByteReader`), per-field limits (text 256, list 64), invalid packet dropped and counted, never given to the game | 0027, 0037 |
| Three kinds of messages | `unreliable` (UDP), `reliable` (UDP + acks, default, in game), `stream` (TCP, out of game) | 0037 |
| Game messages | typed structs encoded by reflection; message ids from registration order | 0036, 0037 |
| Players | `PlayerId`; one name per player; one connection is one player: one TCP stream and one UDP address linked | 0037 |
| Handshake | carries the name and the format fingerprint; refusals `ServerFull`, `FormatMismatch`, invalid name | 0037 |
| Heartbeat and timeout | heartbeat 0.5 s, timeout 5 s; reasons `Quit`, `Timeout`, `Kicked`, `ProtocolError`, `ServerShutdown`, `ServerFull`, `FormatMismatch` | 0033, 0037 |
| Lobby, parties, rooms | client requests become server events; answers become client events; messages routed by where the player is | 0039 |
| Replication | `NetworkId` (never a memory address), spawn by `NetPrefab`, `Replicated` components, 20 snapshots/s, reset when a player changes room | 0033, 0039 |
| Inputs | bit mask of actions sent every tick, `RemoteInput` on the server | 0030, 0033 |
| Entities in messages | sent as `NetworkId`, `Entity::null()` when unknown | 0037 |
| Redirection | a "your room is at this address" message is reserved (dedicated host, Part 2) | 0038 |
| Reconnection | mechanism in the protocol (token, grace period, events), policy in the game | 0040 |
| Authority | the server has the final word; clients send only intentions (inputs, requests) | 0011 |
| Optimizations | the engine gives the means (fields, settings, extension points), the game team optimizes; interpolation is an engine default that can be turned off | 0041 |
| Capacity | `net.maxClients` 32 for the whole server, lobby included; `rooms.maxRooms` 8; 4 players per room | 0038 |

## B. Subject requirements (kick-off slides)

- All in-game communication MUST use UDP. TCP is tolerated with a strong justification (login,
  lobby, text chat), never for in-game traffic.
- The protocol is binary and documented as an RFC. Fixed-width types, one byte order on the wire,
  never raw structs, every length validated, a version field.
- Our own sequence numbers for order, loss and duplicate detection: UDP gives none.
- A malformed packet is dropped, never a crash. A crash never takes the server down, and the other
  players are notified.
- Linux and Windows, interoperable: a Windows client plays on a Linux server.
- The lobby answers a join with the room's address and a session token; the room accepts only a
  valid token; a disconnected player rejoins with it.
- The subject wants numbers: bandwidth used, responsiveness under lag.
- Note: the slides show one process per room; we host rooms in one process first and processes in
  Part 2 (ADR 0038, with its justification).

## C. Budgets

Starting assumptions, to be confirmed or corrected by measurement (step 7).

| Budget | Value | Basis |
|---|---|---|
| Server → client | ≤ 64 KB/s per player, worst case | full R-Type room (~200 entities × ~16 bytes ≈ 3 datagrams) × 20 snapshots/s, no optimization |
| Client → server | ≤ 8 KB/s per player | input (~24 bytes) × 60/s, sent three times for loss, plus heartbeats and messages |
| Datagram | ≤ 1200 bytes | safe size on the Internet (QUIC's reference value) |
| Playable latency | ≤ 150 ms round trip | comfort limit of an action game; interpolation adds ~100 ms |
| Losses | playable at 5 %, never broken at 20 % | average Wi-Fi / robustness test (SRV-05) |
| Duplicates | a packet may arrive twice | subject test condition |
| Whole server | 32 players, ~2 MB/s worst case | `maxClients` 32 × 64 KB/s |
| Constrained link | 150 ms · 2 % loss · duplicates · 5 KB/s | subject test condition (netem / clumsy); the engine must offer enough settings for a game team to stay playable there (ADR 0041) |

A full snapshot of a busy room does not fit in one 1200-byte datagram: decided in step 4.

## D. Threats

| Threat | Example |
|---|---|
| Malformed packet | wrong announced size, out-of-range field, unknown message id |
| UDP spoofing | a third party sends packets with a player's address |
| Flood | thousands of packets per second, or handshakes in a loop |
| Replay and duplicates | an old valid packet sent again, or the network duplicating one |
| Cheating | a client claiming it killed an enemy |
| Different version | a client built with other types |

Out of scope: encryption and key exchange. Not required by the subject, costly, and the game carries
no secret; the RFC states this.
