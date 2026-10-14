# Network part · Step 2: inventory of exchanges

Every exchange, as a sequence diagram, before any byte. For each: direction, when, size, whether it
must arrive, whether order matters, transport. Groups follow a player's life:

| Group | Exchanges | Status |
|---|---|---|
| 1 | Connection: handshake, refusals, TCP ↔ UDP link | validated |
| 2 | Presence: heartbeat, timeout, voluntary leave, kick, shutdown | validated |
| 3 | Lobby and party: invitations, room list | next |
| 4 | Entering and leaving a room | |
| 5 | In game: inputs, snapshots, spawns and destructions, game messages | |
| 6 | End of game, back to the lobby, failed room | |
| 7 | Reconnection | |

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
