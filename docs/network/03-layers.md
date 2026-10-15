# Network part · Step 3: layers

Each exchange of step 2 belongs to one layer. Each layer has one role and knows only the layer below.
The RFC has one section per layer; each layer maps to code already planned; the 37 questions of step 2
sort themselves by layer. Validated on 2026-10-14. Still no bytes.

## The stack

```
┌──────────────────────────────────────────────────────────────────────────┐
│ 5. GAME MESSAGES     PlayRequest, WaveCleared, Chat, results…              │ defined by the game
│                      (ENGINE_MESSAGE types, transport chosen per message)  │ (content only)
├──────────────────────────────────────────────────────────────────────────┤
│ 4. ENGINE MESSAGES                                                         │
│    4a. lobby and party: Invite*, LeaveParty, PartyUpdated, RoomList,       │
│        EnteredRoom, ReturnedToLobby                                        │
│    4b. replication: Snapshot (spawn / update / destroy), Input             │ defined by
├──────────────────────────────────────────────────────────────────────────┤ the engine
│ 3. RELIABILITY (UDP only)  sequences, acks, resends, duplicates, order     │ (the RFC)
├──────────────────────────────────────────────────────────────────────────┤
│ 2. SESSION   Hello, Challenge, UdpLink, Accepted, Refused, Heartbeat, Bye, │
│              Kicked, ServerShutdown, reconnection token                    │
├──────────────────────────────────────────────────────────────────────────┤
│ 1. FRAME     UDP: one datagram = a header + content, ≤ 1200 bytes          │
│              TCP: cut the stream into messages (size, ≤ 64 KB)             │
├──────────────────────────────────────────────────────────────────────────┤
│ 0. TRANSPORT  the OS's UDP and TCP (INetwork plugin)                       │ outside the RFC
└──────────────────────────────────────────────────────────────────────────┘
```

## Role of each layer

| Layer | Single role | Step 4 questions |
|---|---|---|
| 1. Frame | Recognise one of our packets, know its size and kind, drop the rest before reading anything | protocol version (1), aggregation (25), splitting over 1200 bytes (21) |
| 2. Session | Know who speaks: establish, keep and close a player's link, bind TCP and UDP | 1–6 (handshake), 7–11 (presence), 32–37 (reconnection) |
| 3. Reliability | For `reliable` messages: deliver, without duplicates, in the required order; for the others: drop old or duplicated ones | 20 (guaranteed spawns), 22 (snapshot acks), 24 (order) |
| 4a. Lobby and party | Carry the requests and answers of ADR 0039 | 12–15, 16–19, 28–31 |
| 4b. Replication | Snapshots and inputs, with tick, input number and last input processed (ADR 0041) | 3, 21, 23, 26, 27 |
| 5. Game messages | Carry the game's types, encoded by reflection; the game sees only this layer | engine / game message ids |

## Code already planned

| Layer | Files | Tasks |
|---|---|---|
| 0. Transport | `INetwork`, plugin `network_asio` | ENG-36, ENG-37 |
| 1. Frame | `StreamFramer` (TCP), header validation in `NetworkSession` (UDP) | SRV-06, ENG-38 |
| 2. Session | `Handshake`, `PlayerTable`, heartbeats and timeouts in `NetworkSession` | SRV-01, SRV-02, ENG-38 |
| 3. Reliability | `ReliableChannel` | SRV-05 |
| 4a. Lobby | `Lobby`, `ClientLobby`, `PlayerRouting` | SRV-08 to SRV-15 |
| 4b. Replication | `engine.net.*` systems, `NetworkIdMap` | ENG-39 |
| 5. Game messages | `MessageDispatcher`, reflection codec | SRV-03, SRV-04 |

The validated architecture already follows these layers; nothing moves.

## Decisions

1. **Engine messages and game messages use the same mechanism** (layers 4 and 5): typed messages
   encoded by reflection, with separate id ranges; only who declares them differs. One way to encode,
   one to test. The id ranges are decided in step 4.
2. **Reliability exists only on UDP.** TCP already guarantees arrival and order: `stream` messages go
   from layer 1 (TCP framing) straight to layer 4 or 5.
