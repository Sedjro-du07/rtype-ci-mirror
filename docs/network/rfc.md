# RTEP — R-Type Engine Protocol, version 1.0

```
R-Type team                                                    Epitech
Category: Experimental                                    October 2026
Protocol version: 1 (0x01)                       Status: v1.0, frozen
```

## Abstract

This document specifies RTEP, the network protocol of the R-Type engine. A client and a server built
with the engine use it to connect, keep a session alive, gather in a lobby and parties, enter rooms,
replicate a game world and exchange the game's own messages. RTEP runs over UDP for everything that
happens during a game and over TCP for the connection and the lobby. It is binary, big-endian and
defined by the engine for every game; a game only defines the content of its own messages.

Part I specifies the engine protocol. Part II lists the messages of R-Type.

## Status of this memo

Written for the R-Type project (G-ING-500). Version 1.0 is frozen: any change to a byte layout
increments the protocol version (section 4.1). The decisions behind each section are in
[04-decisions.md](04-decisions.md) (D1–D36) and in the ADRs it cites.

## Contents

- Part I · Engine protocol
  1. Introduction
  2. Conventions
  3. Overview
  4. Frame layer
  5. Field encoding
  6. Session layer
  7. Reliability layer
  8. Lobby, party and room messages
  9. Replication
  10. Game messages
  11. Errors and counters
  12. Security considerations
  13. Why TCP is used
  14. Constants and settings
  15. Examples
- Part II · R-Type messages
- References

---

# Part I · Engine protocol

## 1. Introduction

### 1.1 Purpose

RTEP carries everything a networked game made with the engine needs: identity of players, presence,
lobby and parties, rooms (one game each), replication of entities from the server to clients, player
input from clients to the server, and typed messages declared by the game.

### 1.2 Principles

- **The server has the final word.** Clients send intentions (inputs, requests), never results.
- **The engine owns the bytes.** A game declares message types; it never writes or reads bytes.
- **Nothing unvalidated reaches game code.** Every packet and message is checked against this
  specification before any game callback runs; anything else is dropped and counted.
- **Mechanism in the engine, policy in the game.** The protocol carries the data; the game decides
  admission, rules and optimisation settings.

### 1.3 Out of scope

- **Encryption and authentication of the bytes.** Not required by the subject; the game carries no
  secret. Tokens are unpredictable, single use and short-lived (section 12).
- **Congestion control.** Traffic is bounded by per-player budgets (section 9.6) instead.
- **Fragmentation.** No message spans several datagrams (section 4.4).
- **Dedicated room processes.** Reserved for Part 2 of the project (section 8.4).

## 2. Conventions

The key words MUST, MUST NOT, SHOULD, SHOULD NOT and MAY are to be read as described in RFC 2119.

| Notation | Meaning |
|---|---|
| `u8`, `u16`, `u32`, `u64` | unsigned integers of 1, 2, 4, 8 bytes |
| `i32`, `i64` | two's complement signed integers |
| `f32`, `f64` | IEEE 754 binary32 / binary64, sent as their bit pattern |
| `str` | `u16` byte length, then that many bytes of UTF-8 |
| `bytes[n]` | exactly n bytes |
| `list<T>` | `u16` count, then count elements of T |
| C → S, S → C | client to server, server to client |

**All multi-byte values are big-endian** (most significant byte first), on UDP and TCP alike.

| Term | Meaning |
|---|---|
| session | a client known to the server: one TCP stream and one UDP address, bound to a `PlayerId` |
| player | what the game sees of a session (`PlayerId`, name) |
| party | a group of players who play together; every player is in exactly one |
| lobby | all players who are not in a room |
| room | one running game: its own world, tick and players |
| tick | one fixed simulation step of a room (60 per second by default) |
| message | one typed unit with an id; engine messages and game messages share the format |

## 3. Overview

### 3.1 Layers

```
 5  Game messages          the game's types (ENGINE_MESSAGE), ids 0x0100+       section 10
 4  Engine messages        lobby, party, rooms (section 8), replication (9)    ids 0x0001-0x00FF
 3  Reliability            UDP only: packet acks, reliable ordered messages    section 7
 2  Session                handshake, presence, leaving, reconnection          section 6
 1  Frame                  UDP packet header, TCP preamble and frames          section 4
 0  Transport              UDP and TCP of the operating system                 (not specified)
```

### 3.2 Channels

| Channel | Carries | Section |
|---|---|---|
| TCP stream | handshake, session control, lobby and party, game messages declared `stream` | 4.3, 6, 8.1 |
| UDP, unreliable | snapshots, inputs, heartbeats, game messages declared `unreliable` | 9, 6.5 |
| UDP, reliable | room transitions, game messages declared `reliable` | 7.2, 8.3 |

All in-game traffic travels on UDP. Section 13 justifies the use of TCP.

### 3.3 Life of a player

```
connect (TCP) ── Hello ─▶ checks ─▶ Challenge ─▶ UdpLink (UDP) ─▶ Accepted ── lobby, party of 1
    lobby ── invitations, room list ── game asks to play ─▶ EnteredRoom ─▶ RoomReady
    room ── inputs ▲ snapshots ▼ game messages ── ReturnedToLobby ─▶ lobby
    leave ── Bye │ Kicked │ ServerShutdown │ lost ─▶ grace period ─▶ back with the token, or gone
```

## 4. Frame layer

### 4.1 Magic and version

Every UDP datagram and the start of each TCP stream carry:

| Field | Value |
|---|---|
| magic | `0x52 0x54` ("RT") |
| version | `0x01` for this document |

A receiver MUST drop a UDP datagram whose magic or version differs. The **preamble** (magic, version)
and the layout of the `Refused` frame (section 6.3.5) MUST NOT change in any future version, so that a
server can always refuse a client of another version in a way that client understands.

### 4.2 UDP packets

#### 4.2.1 Header (16 bytes)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        magic (0x5254)         |  version (1)  |     kind      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          sessionKey                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           sequence            |              ack              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            ackBits                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     body (0 to 1184 bytes)                    |
```

| Field | Type | Meaning |
|---|---|---|
| magic | u16 | `0x5254` |
| version | u8 | `0x01` |
| kind | u8 | `0x01` UdpLink, `0x02` Data; any other value: drop |
| sessionKey | u32 | Data: the key given in Accepted (6.3.4); UdpLink: 0 |
| sequence | u16 | Data: this packet's number (7.1); UdpLink: 0 |
| ack | u16 | Data: latest sequence received from the peer (7.1); UdpLink: 0 |
| ackBits | u32 | Data: bit i set = sequence `ack − 1 − i` received; UdpLink: 0 |

A datagram MUST NOT exceed **1200 bytes** (`net.maxDatagramBytes`). A receiver MUST drop larger or
shorter-than-header datagrams without allocating.

#### 4.2.2 UdpLink (kind 0x01)

Exactly 24 bytes: the header, then `linkToken u64` (from Challenge, 6.3.2). Sent by the client only.
It binds the sender's UDP address to the session that owns the token (6.3.3, 6.7.3). The server MUST
NOT answer it on UDP.

#### 4.2.3 Data (kind 0x02)

The body is a sequence of zero or more messages, each:

```
+-------------+-------------------------------+-------------+------------------+
|   id u16    | messageSeq u16 (reliable ids) | length u16  |  payload[length] |
+-------------+-------------------------------+-------------+------------------+
```

- `messageSeq` is present only when the message's id is declared **reliable** (sections 7.2, 8, 10);
  both sides know it from the id.
- `length` counts the payload only.
- A Data packet with no message is a **heartbeat** (6.5).

### 4.3 TCP stream

Each side MUST send the 3-byte preamble (magic, version) first. Then the stream is a sequence of
frames:

```
+-------------------+-------------+--------------------------+
|    length u32     |   id u16    |  payload[length − 2]     |
+-------------------+-------------+--------------------------+
```

`length` counts the id and the payload; it MUST be at least 2 and at most **65 536**
(`net.maxStreamMessageBytes`). A larger or smaller announced length MUST close the stream with
`ProtocolError` before reading the payload.

### 4.4 Size limits and no fragmentation

A UDP message MUST fit in one datagram with its packet. A message declared `reliable` or `unreliable`
whose payload exceeds **1024 bytes** MUST be refused by the sender at `send` time, with a log line
naming the message and suggesting `stream`. Snapshots are cut into self-contained parts (9.2).

### 4.5 Message ids

| Range | Owner |
|---|---|
| `0x0000` | invalid: drop the packet or close the stream |
| `0x0001`–`0x00FF` | engine (table in section 8.5) |
| `0x0100`–`0xFFFF` | game: `0x0100 + index` in the order the game registered its messages (section 10) |

Each id has one fixed transport (stream, reliable or unreliable). A message received on a transport
other than its own MUST be treated as invalid.

### 4.6 Validation order (UDP)

A receiver MUST apply these checks in order and drop the datagram, counting the reason, at the first
failure; dropped datagrams are never acknowledged:

1. size between 16 and 1200 bytes;
2. magic and version;
3. kind known; UdpLink: size exactly 24 and a known link token (6.3.3);
4. Data: sender address belongs to a session, and `sessionKey` matches it;
5. sequence not a duplicate and not more than 32 behind the latest received (7.1);
6. every message: id valid, known, allowed in this direction, transport matching, `length` within
   the remaining bytes, payload decoding to exactly `length` bytes (section 5).

Messages are decoded **all before any is delivered**: if one fails, the whole packet is dropped.
Only then are the header's ack fields processed and the messages delivered.

## 5. Field encoding

Messages and replicated components are encoded field by field, in declaration order, with no names,
no padding and no alignment.

| Field kind | Encoding | Validation |
|---|---|---|
| Bool | u8 | 0 or 1, anything else fails |
| Int32, UInt32, Int64 | i32, u32, i64 | — |
| Float, Double | f32, f64 | NaN and infinities fail in replicated components |
| String | str | byte length ≤ field limit (default 256) |
| Vec2 | f32 x, f32 y | as Float |
| Rect | f32 x, y, w, h | as Float |
| Color | u8 r, g, b, a | — |
| Entity | u32 `NetworkId`, 0 = none | unknown id → `Entity::null()` on arrival |
| Texture, Sound, Music, Font, Script | str (asset path relative to the game root) | no `..`, limit 256 |
| Enum | i32 underlying value | unknown value fails |
| List | u16 count, then elements | count ≤ field limit (default 64), checked before allocating |
| Struct | its fields, in order | recursively |
| Custom | the type author's `binary()` codec | the codec's own checks |
| Quantised float (section 9.6) | u16: `round((v − min) / (max − min) × 65535)` | value clamped to [min, max] on send |

A decoder MUST check every length and count against the remaining bytes and the field limit **before**
allocating, and MUST fail rather than read past the end.

## 6. Session layer

### 6.1 State machines

Client:

```
Idle ──connectTo──▶ Connecting ──TCP up, preamble + Hello sent──▶ HelloSent
HelloSent ──Challenge──▶ Linking (UdpLink every 100 ms)
HelloSent / Linking ──Refused──▶ Closed (LeftServer{reason, message})
Linking ──Accepted──▶ Connected (JoinedServer{me} or Reconnected{})
Connected ──Bye sent / Kicked / ServerShutdown──▶ Closed
Connected ──TCP lost or 5 s silence──▶ Interrupted (ConnectionInterrupted{reason})
Interrupted ──auto reconnect: Hello with token──▶ HelloSent
Interrupted ──grace over, or auto reconnect off──▶ Closed (LeftServer{reason})
```

Server, per session:

```
Pending (TCP accepted) ──preamble + Hello, checks pass──▶ Challenged
Pending ──a check fails──▶ Refused, closed
Challenged ──UdpLink with the token──▶ Connected (Accepted; PlayerJoined or PlayerReconnected)
Pending / Challenged ──5 s──▶ Refused(Timeout), closed
Connected ──Bye──▶ Closed (PlayerLeft{Quit})
Connected ──kick / stopServer──▶ Closed (PlayerLeft{Kicked} / ServerShutdown)
Connected ──TCP closed without Bye / 5 s silence──▶ Disconnected (PlayerDisconnected)
Disconnected ──Hello with its token──▶ Challenged (resumed)
Disconnected ──grace over──▶ Closed (PlayerLeft{ConnectionLost | Timeout})
```

### 6.2 Handshake

```
Client                                              Server
  │── TCP connect ──────────────────────────────────▶│
  │── preamble, Hello ─────────────────────── TCP ──▶│  checks (6.4), then the game's admission (6.4.2)
  │◀──────────────────────── preamble, Challenge ────│
  │── UdpLink(linkToken) ── every 100 ms ───── UDP ─▶│  binds the UDP address
  │◀──────────────────────────────────── Accepted ───│
  │◀───────────────────── PartyUpdated, OnlinePlayers│
```

The handshake MUST complete within 5 s of the TCP connection (`net.handshakeSeconds`), else the
server sends `Refused(Timeout)` and closes.

### 6.3 Session messages (TCP)

#### 6.3.1 Hello (0x0001, C → S)

| Field | Type | Meaning |
|---|---|---|
| name | str | player name, 1 to `net.maxNameLength` (16) bytes, no control characters |
| fingerprint | u64 | `TypeRegistry::formatFingerprint()` of the client |
| gameName | str | name of the game module (≤ 32), for example `rtype` |
| reconnectToken | bytes[16] | the last token received in Accepted, or 16 zero bytes |
| gameDataId | u16 | id of the game's admission message (6.4.2), or 0 |
| gameDataLength | u16 | only if gameDataId ≠ 0 |
| gameData | bytes[gameDataLength] | only if gameDataId ≠ 0: that message's payload |

#### 6.3.2 Challenge (0x0002, S → C)

| Field | Type | Meaning |
|---|---|---|
| linkToken | u64 | random; the client echoes it in UdpLink |

#### 6.3.3 Binding the UDP address

When a UdpLink carries the link token of a `Challenged` session, the server records the datagram's
source address as that session's UDP address and sends Accepted. A UdpLink with an unknown token is
dropped silently.

#### 6.3.4 Accepted (0x0003, S → C)

| Field | Type | Meaning |
|---|---|---|
| playerId | u32 | the player's id (never 0) |
| sessionKey | u32 | random; MUST appear in every Data packet of this session, both directions |
| reconnectToken | bytes[16] | new token, single use (6.7) |
| resumed | u8 | 1 if this resumes a disconnected session, else 0 |
| tickRate | u16 | ticks per second of rooms (60) |
| snapshotRate | u16 | snapshots per second (20) |
| heartbeatMs | u16 | 500 |
| timeoutMs | u16 | 5000 |
| graceSeconds | u16 | reconnection grace period (30; 0 = none) |

The client MUST use the server's values. The server then sends `PartyUpdated` (8.1.5) and, if enabled,
`OnlinePlayers` (8.1.8).

#### 6.3.5 Refused (0x0004, S → C)

| Field | Type | Meaning |
|---|---|---|
| reason | u8 | a reason code (6.8) |
| message | str | ≤ 128 bytes, may be empty: the game's or the engine's explanation |

The server closes the stream after sending it. This frame's layout is frozen across versions (4.1).

#### 6.3.6 Bye (0x0005, C → S), Kicked (0x0006, S → C), ServerShutdown (0x0007, S → C), KeepAlive (0x0008, both)

| Message | Payload | Effect |
|---|---|---|
| Bye | — | the client leaves; `PlayerLeft{Quit}`; the server closes |
| Kicked | message str (≤ 128) | `LeftServer{Kicked, message}`; the server closes |
| ServerShutdown | message str (≤ 128) | sent to every session, then closed |
| KeepAlive | — | sent after 10 s without any TCP frame, so NAT devices keep the stream |

### 6.4 Admission

#### 6.4.1 Engine checks

After the preamble, the server MUST check, in this order, and refuse at the first failure:

| # | Check | Refusal |
|---|---|---|
| 1 | preamble magic and version | `ProtocolMismatch` (the rest of Hello is not read) |
| 2 | gameName equals the server's game | `WrongGame` |
| 3 | fingerprint equals the server's | `FormatMismatch` |
| 4 | name valid (length, characters) | `InvalidName` |
| 5 | a token is given and matches a disconnected session | resume that session (6.7) |
| 6 | fewer than `net.maxClients` sessions (32) | `ServerFull` |
| 7 | the game's admission, if any | `RefusedByGame` with the game's message |

#### 6.4.2 The game's admission

The game MAY declare one admission message type (for example `JoinInfo{version}`). The client gives a
value at `connectTo`; it travels in Hello as `gameDataId` and `gameData`. The server's game code
receives the name, the decoded value and whether this is a reconnection, and accepts or refuses with a
text. The decision MUST be synchronous. Without an admission handler, check 7 always passes. It runs
again on reconnection.

### 6.5 Presence

- Each side MUST send a Data packet at least every `heartbeatMs` (500 ms); when it has nothing to send,
  the packet has no message (heartbeat).
- Any valid Data packet resets the peer's silence timer.
- After `timeoutMs` (5 s) without a valid packet, the session is considered lost (6.6).
- RTT is measured from packet acknowledgements (7.1).

### 6.6 Leaving

| Event | Server side | Client side |
|---|---|---|
| client sends Bye | `PlayerLeft{Quit}` at once | `LeftServer{Quit}` |
| server kicks | Kicked, then `PlayerLeft{Kicked}` | `LeftServer{Kicked, message}` |
| server stops | ServerShutdown to all | `LeftServer{ServerShutdown, message}` |
| TCP closed without Bye | `PlayerDisconnected{ConnectionLost}`, grace (6.7) | `ConnectionInterrupted{ConnectionLost}` |
| 5 s without a packet | `PlayerDisconnected{Timeout}`, grace (6.7) | `ConnectionInterrupted{Timeout}` |
| repeated invalid packets (11) | `PlayerLeft{ProtocolError}` at once | `LeftServer{ProtocolError}` |

### 6.7 Reconnection

#### 6.7.1 Grace period

A session lost by `ConnectionLost` or `Timeout` becomes **Disconnected** for `graceSeconds` (30 by
default; 0 = immediate leave). Its `PlayerId`, party and room are kept; `PartyUpdated` marks the member
as not connected; the game receives `PlayerDisconnected{player, reason}` in the lobby and in the room,
and applies its own policy. When the period ends: `PlayerLeft{player, reason}`.

#### 6.7.2 Coming back

The client sends a normal Hello with the reconnection token. If it matches a Disconnected session, the
handshake continues (Challenge, UdpLink) and ends with `Accepted(resumed = 1)` carrying the same
`playerId`, a new session key and a new token. The server then sends `PartyUpdated` and, if the player
was in a room, a new `EnteredRoom` (8.3): replication starts again from nothing. The game receives
`PlayerReconnected{player}`. A token that matches nothing is ignored: the client is treated as new.

#### 6.7.3 UDP address change

When a connected client has received nothing on UDP for 1 s while its TCP stream is open, it MUST send
`UdpLink(linkToken)` with the link token of its current session (every 100 ms until UDP traffic
resumes). The server adopts the new source address.

#### 6.7.4 Automatic reconnection

A client in `Interrupted` tries again every 2 s until the grace period announced in Accepted is over
(`net.autoReconnect`, default on).

### 6.8 Reason codes

| Code | Name | Meaning |
|---|---|---|
| 0 | Quit | the client left (Bye) |
| 1 | Timeout | no packet for `timeoutMs`, or handshake too slow |
| 2 | Kicked | removed by the game |
| 3 | ProtocolError | invalid traffic (section 11) |
| 4 | ServerShutdown | the server stopped |
| 5 | ServerFull | `net.maxClients` reached |
| 6 | FormatMismatch | the game's types differ: rebuild client and server from the same version |
| 7 | ProtocolMismatch | the engine's protocol version differs: update the engine |
| 8 | WrongGame | the server runs another game |
| 9 | InvalidName | name empty, too long or with control characters |
| 10 | RefusedByGame | the game's admission refused; the message says why |
| 11 | ConnectionLost | the TCP stream closed without Bye |

## 7. Reliability layer (UDP)

### 7.1 Packet sequence and acknowledgements

- Each side numbers its Data packets from 1, adding 1 per packet, wrapping at 2¹⁶. Sequence `a` is
  newer than `b` when `(a − b) mod 65536` is between 1 and 32767.
- `ack` is the newest sequence received from the peer; bit i of `ackBits` (bit 0 = least significant)
  says whether `ack − 1 − i` was received. Every Data packet carries them, so each packet is
  acknowledged up to 33 times.
- A packet with a sequence already received, or more than 32 behind the newest received, MUST be
  dropped (it cannot be acknowledged any more).
- Acknowledgements for sequences not yet sent MUST be ignored.
- If a packet carrying a reliable message was received and nothing is sent within 33 ms, the receiver
  MUST send a Data packet without message, to carry the ack.
- RTT: on each newly acknowledged packet, `rtt = 0.9 × rtt + 0.1 × sample`. Loss: the share of the
  last 64 sent packets not acknowledged after 1 s.

### 7.2 Reliable messages

- Each reliable message has a `messageSeq` (u16, per direction, from 0, wrapping).
- The sender remembers which reliable messages each packet carried. When a packet is acknowledged,
  its messages are delivered. When it is not acknowledged after `clamp(1.5 × rtt, 50 ms, 1000 ms)`,
  its unacknowledged messages are sent again in the next packet, with the same `messageSeq`.
- At most 256 reliable messages MAY be unacknowledged; more wait in a local queue.
- Delivery is **in `messageSeq` order**, one ordered stream per direction. The receiver holds up to 256
  messages ahead of the next expected one; beyond, it drops them (they will be resent). Duplicates are
  dropped.
- A reliable message is never abandoned; only the end of the session ends it.

### 7.3 Unreliable messages

Sent once, never resent. Duplicates are removed by 7.1. Snapshot and Input apply their own "newest
wins" rule (9.2, 9.4).

## 8. Lobby, party and room messages

The client sends requests; the server turns them into events for the game, which decides through the
lobby API; the engine applies the decision and informs the clients.

### 8.1 Lobby and party (TCP)

#### 8.1.1 InviteRequest (0x0020, C → S)

`to u32` (player to invite). Server event `InviteRequested{from, to}`.

#### 8.1.2 InviteReceived (0x0021, S → C)

`from u32 · fromName str · party u32`. Sent when the game calls `sendInvite`. Delivered even during a
game; the client game decides whether to show it.

#### 8.1.3 InviteAnswer (0x0022, C → S)

`party u32 · accept u8`. Server event `InviteAnswered{player, party, accepted}`.

#### 8.1.4 LeaveParty (0x0023, C → S)

No payload. Server event `LeavePartyRequested{player}`.

#### 8.1.5 PartyUpdated (0x0024, S → C)

`party u32 · leader u32 · room u32 (0 = lobby) · members list<{player u32 · name str · connected u8}>`.
Sent to every member after any change (join, leave, new leader, disconnection, resumption). The latest
one replaces the previous state. A party always has a leader; a leader who is disconnected stays
leader during the grace period (the game may change it with `setLeader`) and is replaced only when they
leave for good.

#### 8.1.6 RoomListRequest (0x0025, C → S)

No payload. Server event `RoomListRequested{player}`.

#### 8.1.7 RoomList (0x0026, S → C)

`rooms list<{room u32 · name str · state u8 · players u8 · maxPlayers u8 · listed u8}>`, at most 64.
The game chooses which rooms to send.

#### 8.1.8 OnlinePlayers (0x0027), PlayerOnline (0x0028), PlayerOffline (0x0029), S → C

`OnlinePlayers`: `players list<{player u32 · name str}>`, sent after Accepted. `PlayerOnline`:
`player u32 · name str`. `PlayerOffline`: `player u32`. Sent to clients in the lobby unless the game
disables them (`net.shareOnlinePlayers`).

#### 8.1.9 RequestRefused (0x002A, S → C)

`request u8 (1 Invite, 2 InviteAnswer, 3 LeaveParty, 4 RoomList, 5 Game) · reason str (≤ 128)`. Sent
when the game calls `Lobby::refuse`.

### 8.2 Starting and joining a game

Starting, joining or leaving a game is a rule of the game, so it is a **game message** (Part II), not
an engine message. The game answers by calling `createRoom` / `moveParty` / `moveToLobby`, which
produce the room messages below.

### 8.3 Room transitions (reliable UDP)

These messages travel in the reliable ordered UDP stream (7.2), so they stay ordered with the game's
reliable messages.

#### 8.3.1 EnteredRoom (0x0040, S → C)

| Field | Type | Meaning |
|---|---|---|
| room | u32 | room id |
| roomSession | u16 | new for each entry of this player into a room |
| name | str | room name (≤ 32) |
| level | str | first scene of the room |
| maxPlayers | u8 | |
| listed | u8 | 1 public, 0 private |
| roomTick | u32 | the room's current tick |
| tickRate | u16 | ticks per second |

The client changes scene, then sends RoomReady.

#### 8.3.2 RoomReady (0x0041, C → S)

`roomSession u16`. The server starts replication for this player (spawns of everything relevant) only
after RoomReady with the current `roomSession`.

#### 8.3.3 ReturnedToLobby (0x0042, S → C)

`room u32 · reason str (≤ 128)`. Sent on `moveToLobby` or `closeRoom`; the reason is the game's. The
client goes back to its lobby scene; snapshots of that room are dropped from now on.

#### 8.3.4 RoomInterrupted (0x0043, S → C)

`room u32`. Sent as soon as the room fails (a system threw). The session stays alive; the game decides
what follows (usually `closeRoom`, then ReturnedToLobby).

#### 8.3.5 RoomRedirect (0x0044, reserved)

Reserved for dedicated room processes (Part 2): `address str · port u16 · token bytes[16]`. A version 1
peer MUST treat it as invalid.

### 8.4 Ordering at the end of a game

Results are a game message sent `reliable` before `closeRoom`; since ReturnedToLobby travels in the
same ordered stream, results always arrive first.

### 8.5 Engine message ids

| Id | Name | Direction | Transport | Section |
|---|---|---|---|---|
| 0x0001 | Hello | C → S | TCP | 6.3.1 |
| 0x0002 | Challenge | S → C | TCP | 6.3.2 |
| 0x0003 | Accepted | S → C | TCP | 6.3.4 |
| 0x0004 | Refused | S → C | TCP | 6.3.5 |
| 0x0005 | Bye | C → S | TCP | 6.3.6 |
| 0x0006 | Kicked | S → C | TCP | 6.3.6 |
| 0x0007 | ServerShutdown | S → C | TCP | 6.3.6 |
| 0x0008 | KeepAlive | both | TCP | 6.3.6 |
| 0x0020 | InviteRequest | C → S | TCP | 8.1.1 |
| 0x0021 | InviteReceived | S → C | TCP | 8.1.2 |
| 0x0022 | InviteAnswer | C → S | TCP | 8.1.3 |
| 0x0023 | LeaveParty | C → S | TCP | 8.1.4 |
| 0x0024 | PartyUpdated | S → C | TCP | 8.1.5 |
| 0x0025 | RoomListRequest | C → S | TCP | 8.1.6 |
| 0x0026 | RoomList | S → C | TCP | 8.1.7 |
| 0x0027 | OnlinePlayers | S → C | TCP | 8.1.8 |
| 0x0028 | PlayerOnline | S → C | TCP | 8.1.8 |
| 0x0029 | PlayerOffline | S → C | TCP | 8.1.8 |
| 0x002A | RequestRefused | S → C | TCP | 8.1.9 |
| 0x0040 | EnteredRoom | S → C | UDP reliable | 8.3.1 |
| 0x0041 | RoomReady | C → S | UDP reliable | 8.3.2 |
| 0x0042 | ReturnedToLobby | S → C | UDP reliable | 8.3.3 |
| 0x0043 | RoomInterrupted | S → C | UDP reliable | 8.3.4 |
| 0x0044 | RoomRedirect | S → C | reserved | 8.3.5 |
| 0x0060 | Snapshot | S → C | UDP unreliable | 9.2 |
| 0x0061 | Input | C → S | UDP unreliable | 9.4 |

All other ids from 0x0001 to 0x00FF are reserved for future engine versions.

## 9. Replication

### 9.1 Identity

Replicated entities are named by a `NetworkId` (u32, never 0, never reused during a server's life),
assigned by the server. A client spawns an entity from the prefab named by its `prefabKey` (fnv1a of
the prefab path). Only components flagged `Replicated` travel; each is named by its
`componentIndex`: its position among `Replicated` components in registration order (engine first),
identical on both sides by the fingerprint.

### 9.2 Snapshot (0x0060, S → C, unreliable)

| Field | Type | Meaning |
|---|---|---|
| roomSession | u16 | the receiving player's current room session |
| tick | u32 | the room tick this state belongs to |
| lastInputProcessed | u32 | newest input of this player included in this state (9.4) |
| count | u16 | number of entries |
| entries | count × entry | |

Entry:

| Field | Type | Present |
|---|---|---|
| networkId | u32 | always |
| op | u8 | always: 0 update, 1 spawn, 2 destroy |
| prefabKey | u32 | spawn |
| componentCount | u8 | spawn, update |
| components | componentCount × (componentIndex u16 · length u16 · fields) | spawn, update |

Receiver rules:

- a snapshot whose `roomSession` is not the current one MUST be dropped;
- an entry is applied only if `tick` is newer than the last tick applied to that entity;
- an update for an unknown entity is ignored (its spawn will come);
- a spawn for a known entity is applied as an update;
- the client remembers destroyed ids (and destroys of ids it never saw) for the room session and
  ignores every later entry for them;
- the client keeps the two latest states of each entity for interpolation (9.5).

### 9.3 What the server sends: until acknowledged

For each client the server keeps, per entity, whether the client acknowledged its spawn, and for each
component a hash of the last bytes the client acknowledged. In each snapshot it sends:

- a **spawn** (with all replicated components) for each relevant entity whose spawn is not yet
  acknowledged;
- a **destroy** for each entity destroyed or no longer relevant whose destroy is not yet acknowledged;
- an **update** with the components whose current bytes differ from the acknowledged ones.

An item is acknowledged when a packet that carried it is acknowledged (7.1). A lost snapshot costs
nothing more than sending its content again. Several Snapshot messages MAY carry the same tick, each
self-contained, to stay within 1024 bytes per message (4.4).

### 9.4 Input (0x0061, C → S, unreliable)

| Field | Type | Meaning |
|---|---|---|
| roomSession | u16 | current room session; otherwise dropped |
| newestSeq | u32 | number of the newest input in this message (client counter, from 1) |
| count | u8 | 1 to 4 |
| masks | count × u64 | action masks, newest first: `newestSeq`, `newestSeq − 1`, … |

- The client sends its input every tick, with the last `net.inputRedundancy` (3) inputs.
- The server keeps up to 4 pending inputs per player, ignores those already processed, applies **one
  per room tick** in order into `RemoteInput`, and when more than 4 wait drops the oldest.
- Bits of a mask beyond the registered actions MUST be ignored.
- `lastInputProcessed` in that player's snapshots is the last input applied.

### 9.5 Time and interpolation

`EnteredRoom` gives the room tick and tick rate; snapshots give their tick. The client estimates the
server tick as `latest snapshot tick + (rtt / 2) × tickRate` and, by default, draws other entities
`net.interpolationDelay` (0.1 s) in the past, between the two states around that time. The game MAY
disable this default (`engine.net.interpolate`) and draw its own, and MAY predict its own entities
using `lastInputProcessed` (ADR 0041).

### 9.6 Budget, priority, relevance, precision

- **Budget:** snapshot bytes to a player per second MUST NOT exceed `net.sendBudgetBytesPerSecond`
  (65 536). When the budget is reached, remaining entries wait for the next snapshot.
- **Priority:** each entity accumulates its `NetPriority.weight` (default 1) every snapshot it misses;
  entries are written in decreasing accumulated priority, which resets when sent. Nobody starves.
- **Relevance:** the game MAY give a room a relevance function (player, entity → bool); an entity that
  becomes irrelevant is destroyed on that client (9.3).
- **Precision:** a float or Vec2 field declared `.quantize(min, max)` travels as u16 (section 5).

### 9.7 Order inside a packet

A server packet puts snapshot messages before other messages, and a receiver applies them first, so a
message travelling with a spawn finds its entity. Otherwise an unknown `Entity` field becomes
`Entity::null()` (section 5).

## 10. Game messages

- A game declares its messages with `ENGINE_MESSAGE` and registers them; message `i` in registration
  order gets id `0x0100 + i`. Client and server MUST load the same game build: the fingerprint
  (6.4.1) guarantees the ids, fields and transports agree.
- Each message has one transport: `stream` (TCP), `reliable` (UDP, 7.2) or `unreliable` (UDP, 7.3).
- Payloads follow section 5, with the field limits declared by the game (`maxLength`).
- In a room, a game message from a player is delivered to that room's code; outside a room, to the
  lobby code. Messages a room sends to a player who has left it are dropped.
- The game's admission message (6.4.2) is a game message carried inside Hello.

## 11. Errors and counters

| Situation | Action | Counter |
|---|---|---|
| datagram fails 4.6 checks 1–5 | drop, no answer | `net.dropped.{size,magic,kind,session,sequence}` |
| message fails check 6 | drop the whole packet | `net.dropped.message` |
| more than 50 dropped packets from a session in 10 s | close with `ProtocolError` | `net.closed.protocol` |
| more than 300 packets per second from a session | drop the excess | `net.dropped.rate` |
| TCP frame length out of range, unknown or misplaced id, payload not decoding | close with `ProtocolError` | `net.closed.protocol` |
| more than 16 pending handshakes, or 4 from one address | close the new TCP connection at once | `net.refused.pending` |
| inbox full (`net.maxQueuedDatagrams`) | drop | `net.dropped.inbox` |

Counters are visible with the console command `net` and in the metrics overlay. No error ever reaches
game code; a game callback that throws is caught and logged (ADR 0037).

## 12. Security considerations

| Threat | Countermeasure |
|---|---|
| malformed packet | validation order 4.6 before any use; bounded reads; limits checked before allocation; all-or-nothing packet decoding |
| UDP spoofing (blind) | UDP address bound only through a link token received on TCP; `sessionKey` in every Data packet |
| flood of packets | per-session rate cap, invalid-packet limit, bounded inbox; no answer to unauthenticated UDP (no amplification) |
| flood of handshakes | 16 pending, 4 per address, 5 s deadline |
| replay and duplicates | sequence window (7.1), reliable `messageSeq`, newest-wins for snapshots and inputs, `roomSession` |
| cheating | server authority: clients send only inputs and requests; unknown action bits ignored; the game validates requests |
| different version | `ProtocolMismatch`, `WrongGame`, `FormatMismatch` before anything else |
| token theft | tokens from the OS secure random source, single use, bound to the player, dead after the grace period |

**Not covered:** an attacker who can read the traffic (same Wi-Fi, a router on the path) can read
tokens and session keys and impersonate a player. Protecting against that needs encryption (DTLS or a
QUIC-like scheme), out of scope for version 1: the subject does not require it and the game carries
no secret.

## 13. Why TCP is used

The subject requires UDP for all in-game communication and tolerates TCP with a strong justification.
RTEP uses TCP **only outside the simulation**:

| Use | Why TCP |
|---|---|
| handshake | the link token and reconnection token must arrive intact and in order before UDP is trusted; TCP gives this without a second reliability scheme |
| lobby and party | requests and lists must arrive and in order, with no real-time constraint; room lists can exceed one datagram |
| session control (Bye, Kicked, ServerShutdown, KeepAlive) | a closed TCP stream also tells at once that a client crashed |
| game messages declared `stream` | text chat and other non-simulation data chosen by the game |

Everything that belongs to a running game travels on UDP: snapshots, inputs, room transitions and the
game's `reliable` and `unreliable` messages.

## 14. Constants and settings

| Name | Default | Section |
|---|---|---|
| magic, version | `0x5254`, `0x01` | 4.1 |
| `net.port` | 4242 (TCP and UDP) | — |
| `net.maxDatagramBytes` | 1200 | 4.2 |
| UDP message payload max | 1024 | 4.4 |
| `net.maxStreamMessageBytes` | 65 536 | 4.3 |
| `net.maxClients` | 32 | 6.4 |
| `net.maxNameLength` | 16 | 6.3.1 |
| `net.handshakeSeconds` | 5 | 6.2 |
| `net.maxPendingHandshakes` | 16 (4 per address) | 11 |
| UdpLink resend | every 100 ms | 6.2 |
| `net.heartbeatSeconds` | 0.5 | 6.5 |
| `net.timeoutSeconds` | 5 | 6.5 |
| TCP KeepAlive | after 10 s idle | 6.3.6 |
| `net.reconnectGraceSeconds` | 30 | 6.7 |
| `net.autoReconnect` | on, every 2 s | 6.7.4 |
| UDP relink after silence | 1 s | 6.7.3 |
| ack window | 33 packets | 7.1 |
| ack-only delay | 33 ms | 7.1 |
| reliable resend | `clamp(1.5 × rtt, 50 ms, 1 s)` | 7.2 |
| reliable window | 256 | 7.2 |
| `net.snapshotRate` | 20 | 9 |
| `net.sendBudgetBytesPerSecond` | 65 536 | 9.6 |
| `net.inputRedundancy` | 3 | 9.4 |
| pending inputs per player | 4 | 9.4 |
| `net.interpolationDelay` | 0.1 s | 9.5 |
| invalid packets before closing | 50 in 10 s | 11 |
| packets per second per session | 300 | 11 |
| `net.shareOnlinePlayers` | on | 8.1.8 |
| string limit, list limit | 256 bytes, 64 elements | 5 |

## 15. Examples

The values are illustrative (tokens, keys, fingerprint); the layouts are exact.

### 15.1 Connection of "Sedjro" to an R-Type server

Client → server, TCP: preamble and Hello.

```
52 54 01                                  preamble: "RT", version 1
00 00 00 2B                               frame length 43 (id + 41 bytes of payload)
00 01                                     id 0x0001 Hello
00 06 53 65 64 6A 72 6F                   name "Sedjro"
1A 2B 3C 4D 5E 6F 70 81                   fingerprint
00 05 72 74 79 70 65                      gameName "rtype"
00 00 00 00 00 00 00 00                   reconnectToken: none (16 zero bytes)
00 00 00 00 00 00 00 00
00 00                                     gameDataId 0: no admission data
```

Server → client, TCP: preamble and Challenge.

```
52 54 01                                  preamble
00 00 00 0A  00 02                        length 10, id 0x0002 Challenge
9F 3C 2A 77 10 B4 E8 5D                   linkToken
```

Client → server, UDP (24 bytes), repeated every 100 ms until Accepted:

```
52 54 01 01                               magic, version, kind 0x01 UdpLink
00 00 00 00  00 00  00 00  00 00 00 00    sessionKey, sequence, ack, ackBits: 0
9F 3C 2A 77 10 B4 E8 5D                   linkToken
```

Server → client, TCP: Accepted, PartyUpdated, OnlinePlayers.

```
00 00 00 25  00 03                        length 37, id 0x0003 Accepted
00 00 00 07                               playerId 7
C4 1D 8E 23                               sessionKey
5E 0B 91 C7 22 4A F0 13                   reconnectToken
8D 66 B2 09 E4 71 3C A8
00                                        resumed: no
00 3C  00 14  01 F4  13 88  00 1E         tickRate 60, snapshotRate 20, heartbeat 500 ms,
                                          timeout 5000 ms, grace 30 s

00 00 00 1D  00 24                        length 29, id 0x0024 PartyUpdated
00 00 00 03  00 00 00 07  00 00 00 00     party 3, leader 7, room 0 (lobby)
00 01                                     1 member:
00 00 00 07  00 06 53 65 64 6A 72 6F  01    player 7, "Sedjro", connected

00 00 00 10  00 27                        length 16, id 0x0027 OnlinePlayers
00 01  00 00 00 07  00 06 53 65 64 6A 72 6F   1 player: 7 "Sedjro"
```

A refusal for a full server would have been:

```
00 00 00 05  00 04  05  00 00             length 5, Refused, reason 5 ServerFull, empty message
```

### 15.2 A snapshot in a room (72 bytes)

Server → client, UDP. Room session 1, tick 1200; the player's input 1234 is included. Bydos 57 spawns
(only `Position` is replicated in this example, component index 0), ship 18 moves.

```
52 54 01 02                               magic, version, kind 0x02 Data
C4 1D 8E 23                               sessionKey
00 7B                                     sequence 123
01 2B  FF FF FF FF                        ack 299, and the 32 before it
00 60  00 34                              id 0x0060 Snapshot (unreliable: no messageSeq), length 52
00 01  00 00 04 B0  00 00 04 D2           roomSession 1, tick 1200, lastInputProcessed 1234
00 02                                     2 entries
00 00 00 39  01  8C 2F 11 A0              entity 57, spawn, prefabKey
01  00 00  00 08  43 B4 00 00 42 F0 00 00   1 component: index 0 Position, 8 bytes: (360.0, 120.0)
00 00 00 12  00                           entity 18, update
01  00 00  00 08  42 20 00 00 42 B4 00 00   1 component: Position (40.0, 90.0)
```

### 15.3 An input (51 bytes)

Client → server, UDP: input 1234 (up + fire, mask 5) sent with the two before it.

```
52 54 01 02  C4 1D 8E 23                  Data, sessionKey
01 2C  00 7B  FF FF FF FF                 sequence 300, ack 123 and the 32 before it
00 61  00 1F                              id 0x0061 Input, length 31
00 01  00 00 04 D2  03                    roomSession 1, newestSeq 1234, 3 inputs
00 00 00 00 00 00 00 05                   #1234: bits 0 (up) and 2 (fire)
00 00 00 00 00 00 00 05                   #1233
00 00 00 00 00 00 00 01                   #1232: up
```

### 15.4 A reliable game message (26 bytes)

Server → client: R-Type's `WaveCleared{3}` (id 0x0105, Part II), reliable message number 5.

```
52 54 01 02  C4 1D 8E 23                  Data, sessionKey
00 7C  01 2C  FF FF FF FF                 sequence 124, ack 300
01 05  00 05  00 04                       id 0x0105, messageSeq 5, length 4
00 00 00 03                               wave 3
```

If packet 124 is not acknowledged within 1.5 × RTT, the same message (`messageSeq` 5) goes again in a
later packet; the client delivers it once, in order.

### 15.5 A heartbeat (16 bytes)

```
52 54 01 02  C4 1D 8E 23  00 7D  01 2C  FF FF FF FF      Data with no message
```

---

# Part II · R-Type messages

R-Type registers its messages in the order below, which gives their ids (section 10). This list is
**provisional**: the Game part of the architecture completes it; any change to it changes the
fingerprint, never the engine protocol.

| Id | Message | Fields | Transport | Direction | Use |
|---|---|---|---|---|---|
| 0x0100 | `JoinInfo` | `version str (≤ 16)` | inside Hello | C → S | admission: the server refuses an older game version |
| 0x0101 | `PlayRequest` | — | stream | C → S | the party leader starts a game; the game creates a room |
| 0x0102 | `JoinRoomRequest` | `room u32` | stream | C → S | join a listed room with free places |
| 0x0103 | `LeaveGame` | — | reliable | C → S | leave the current game (party back to the lobby) |
| 0x0104 | `Chat` | `text str (≤ 128)` | stream | both | text chat, lobby and game |
| 0x0105 | `WaveCleared` | `wave i32` | reliable | S → C | banner between waves |
| 0x0106 | `PlayerKilled` | `victim Entity` | reliable | S → C | death effect, score |
| 0x0107 | `GameResult` | `victory bool · score i32` | reliable | S → C | results screen, sent before `closeRoom` (8.4) |

Gameplay state (ships, Bydos, missiles, the Force, score) travels as replicated components, not as
messages; the Game part lists them.

---

## References

- RFC 2119, key words for use in RFCs to indicate requirement levels.
- RFC 768, User Datagram Protocol. RFC 9293, Transmission Control Protocol.
- RFC 9000, QUIC (1200-byte minimum datagram size as a safe payload).
- G. Fiedler, "Reliability and Ordering over UDP", "Packet Fragmentation and Reassembly", gafferongames.com.
- G. Gambetta, "Fast-Paced Multiplayer".
- Valve, "Source Multiplayer Networking".
- R-Type architecture: ADR 0033, 0037–0043; [04-decisions.md](04-decisions.md).
