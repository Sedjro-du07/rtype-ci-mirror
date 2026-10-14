# Network part · Step 6: validation on scenarios

The RFC walked through hard cases, on paper. Each scenario must have its answer written in the RFC;
the right column gives the section. A scenario without an answer would have sent us back to step 4:
none did, but four needed a precision, added to the RFC and listed at the end.

## Connection

| # | Scenario | What happens | RFC |
|---|---|---|---|
| S1 | Normal connection | Hello → checks → Challenge → UdpLink → Accepted, PartyUpdated (party of 1), OnlinePlayers; `JoinedServer` / `PlayerJoined` | 6.2, 15.1 |
| S2 | Client built from another engine version | server reads only the preamble, answers `Refused(ProtocolMismatch)` in the frozen layout; client logs "update the engine" | 4.1, 6.4.1 |
| S3 | Pong client on the R-Type server | `Refused(WrongGame)` before the fingerprint, so the message is not "rebuild both" | 6.4.1 |
| S4 | Same game, a component changed on one side | `Refused(FormatMismatch)` | 6.4.1 |
| S5 | 33rd player | `Refused(ServerFull)` | 6.4.1 |
| S6 | Game refuses an old client (`JoinInfo{version}`) | `Refused(RefusedByGame, "update to 1.3")` | 6.4.2 |
| S7 | UDP blocked by a firewall | UdpLink never arrives; after 5 s `Refused(Timeout)`; client log suggests UDP | 6.2 |
| S8 | Client closes during the handshake | session dropped; no game event (never accepted) | 6.1 |

## Hostile traffic

| # | Scenario | What happens | RFC |
|---|---|---|---|
| S9 | Random bytes on the UDP port | dropped at magic or size; counted; no answer | 4.6, 11 |
| S10 | Valid header, message length larger than the rest | whole packet dropped, nothing delivered, not acknowledged | 4.6, 11 |
| S11 | List announcing 4 billion elements | count checked against the limit (64) before allocating; packet dropped | 5 |
| S12 | Third party sends packets with a player's address | `sessionKey` does not match (blind attacker): dropped | 4.6, 12 |
| S13 | 1000 TCP connections that never send Hello | 16 pending at most, 4 per address, each closed after 5 s | 11, 12 |
| S14 | Connected client floods 10 000 packets/s | excess over 300/s dropped; if they are invalid, closed after 50 in 10 s | 11 |
| S15 | Old valid packet replayed | sequence duplicate or out of window: dropped | 7.1 |
| S16 | Client sends mask bits of actions that do not exist | bits ignored | 9.4 |
| S17 | Client claims a kill with a game message | the game's server code validates; the engine never applies client results | 1.2, 12 |
| S18 | TCP frame announcing 1 GB | length > 65 536: stream closed with `ProtocolError` before reading | 4.3 |

## Degraded network

| # | Scenario | What happens | RFC |
|---|---|---|---|
| S19 | 20 % loss in game | lost snapshot content goes again in the next one; lost inputs covered by redundancy (3 copies); reliable messages resent; spawns and destroys sent until acknowledged | 7, 9.3, 9.4 |
| S20 | Packets reordered | snapshot entries applied only if newer per entity; inputs applied in order from the buffer; reliable messages delivered in order | 7.2, 9.2, 9.4 |
| S21 | Packets duplicated (netem) | duplicate sequence dropped before any message | 7.1 |
| S22 | Spawn packet overtaken by the destroy packet | destroy remembered for the room session; the late spawn is ignored: no ghost | 9.2 |
| S23 | 150 ms RTT | interpolation 100 ms behind; inputs applied ~75 ms after sending; game may predict with `lastInputProcessed` | 9.5 |
| S24 | Link limited to 5 KB/s (subject test) | game sets `net.sendBudgetBytesPerSecond` ≈ 4096, quantises positions, sets priorities: important entities first, the rest later; only changed components travel | 9.3, 9.6 |
| S25 | A busy tick does not fit in one datagram | snapshot split into self-contained parts; budget and priority decide what waits | 4.4, 9.3 |

## Lobby and rooms

| # | Scenario | What happens | RFC |
|---|---|---|---|
| S26 | Invitation refused by the game (not leader) | `RequestRefused(Invite, "only the leader invites")` if the game calls `refuse`; silence otherwise | 8.1.9 |
| S27 | Party starts a game | `PlayRequest` → game → room; `EnteredRoom` (reliable UDP) → client loads → `RoomReady` → spawns stream in | 8.3 |
| S28 | First snapshot would arrive before the client knows the room | impossible: snapshots start after `RoomReady` of the current `roomSession` | 8.3.2 |
| S29 | Snapshots of the old room after ReturnedToLobby | other `roomSession`: dropped | 8.3.3, 9.2 |
| S30 | Results then return to the lobby | results (reliable) and ReturnedToLobby share one ordered stream: results first | 8.4 |
| S31 | A system throws in room 7 | `RoomInterrupted` at once; heartbeats continue (no timeout); game closes the room; `ReturnedToLobby` with its reason; other rooms untouched | 8.3.4 |
| S32 | `maxRooms` reached | `createRoom` fails; the game tells its player with its own message (or `refuse(Game, …)`) | 8.1.9, 8.2 |
| S33 | Server stops during games | `ServerShutdown` to all, then closed | 6.6 |

## Reconnection

| # | Scenario | What happens | RFC |
|---|---|---|---|
| S34 | Wi-Fi blip of 2 s | under the 5 s timeout: nothing visible but lag | 6.5 |
| S35 | Box changes the UDP port (NAT rebinding) | client sees 1 s of UDP silence with TCP up, sends UdpLink again; server adopts the new address | 6.7.3 |
| S36 | Cable pulled 10 s in game | server: `PlayerDisconnected{Timeout}`, game policy (for R-Type: ship kept, invulnerable); client reconnects by itself with its token: same `PlayerId`, party, `EnteredRoom` again, full state | 6.7 |
| S37 | Back after 40 s | grace over at 30 s: `PlayerLeft{Timeout}`; the token matches nothing: new player | 6.7 |
| S38 | Client crashed and restarted | token was in memory: new player; the old session ends with its grace period | 6.7 |
| S39 | Party leader disconnects | `PartyUpdated` marks them not connected; leader changes only when they leave for good | 6.7.1, 8.1.5 |
| S40 | Player banned while disconnected | admission runs again on reconnection: `Refused(RefusedByGame)` | 6.4.2 |

## Precisions added to the RFC while walking

1. **Refusing another version.** S2 showed the server must answer a client whose bytes it may not
   understand: the preamble and the `Refused` frame are frozen across versions (4.1).
2. **All-or-nothing packets.** S10: messages are decoded all before any is delivered, and a dropped
   packet is never acknowledged, so its reliable content is resent (4.6).
3. **Ghost entities.** S22: destroyed ids are remembered for the room session (9.2).
4. **Leader during the grace period.** S39: a disconnected leader stays leader until they leave for
   good; the game may change it earlier with `setLeader`.
