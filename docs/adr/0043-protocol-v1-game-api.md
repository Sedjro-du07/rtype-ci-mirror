# ADR 0043: Protocol v1: what the game sees of it

- Status: Accepted (completes [ADR 0037](0037-game-facing-networking.md), [0039](0039-room-api.md), [0041](0041-netcode-means-in-engine-policy-in-game.md), [0042](0042-admission-by-the-game.md))
- Date: 2026-10-14
- Deciders: the R-Type team (Network part, step 4, choices delegated to the architect's recommendations)
- Discussion id: NET-3 (D1–D36 in `docs/network/04-decisions.md`)

## Context

The protocol RFC v1.0 (`docs/network/rfc.md`) fixes the bytes. Most of it stays inside the engine,
but some decisions change what a game can call, receive or configure: new reasons for leaving,
reconnection, admission, the online players list, refused requests, room transitions, and the
optimisation settings promised by ADR 0041.

## Decision

1. **Reasons.** `DisconnectReason` gains `ProtocolMismatch`, `WrongGame`, `InvalidName`,
   `RefusedByGame`, `ConnectionLost`. `PlayerLeft` and `LeftServer` gain a `message` (text of the
   game or the engine: kick reason, refusal reason).
2. **Admission (ADR 0042).** Client: `connectTo(host, port, name, data)` with any game message as
   `data`. Server lobby code: `network().admission<T>(fn)`, `fn(const AdmissionRequest&, const T&)`
   returns `Admission::accept()` or `Admission::refuse(text)`; synchronous; runs again on reconnection.
3. **Reconnection.** Server events `PlayerDisconnected{player, reason}` and `PlayerReconnected{player}`
   (lobby, and the room the player is in); `PlayerLeft` comes when the grace period ends. Client
   events `ConnectionInterrupted{reason}` and `Reconnected{}`; the engine reconnects by itself.
   Settings `net.reconnectGraceSeconds` (30) and `net.autoReconnect` (on).
4. **Lobby.** `Lobby::refuse(player, LobbyRequest, reason)` → client event `RequestRefused{request,
   reason}`. `closeRoom(room, reason = "")`, `moveToLobby(party, reason = "")`. `ClientLobby` gains
   `onlinePlayers()`, `name(player)`, `connected(player)` and the event `OnlinePlayersChanged`;
   setting `net.shareOnlinePlayers` (on). Client event `RoomInterrupted{room}` when a room fails.
5. **Room entry.** The client engine sends `RoomReady` by itself once the scene switch requested while
   handling `EnteredRoom` has completed (or at the end of that frame if none); replication starts
   then. Nothing to write in the game.
6. **Optimisation settings (ADR 0041).** `net.sendBudgetBytesPerSecond` (65 536), component
   `NetPriority{weight}`, `RoomNetwork::setRelevance(fn)`, field option `.quantize(min, max)` on float
   and `Vec2` fields, `net.interpolationDelay` (0.1 s), `net.inputRedundancy` (3), and the existing
   `net.snapshotRate` and `engine.net.interpolate` switch.
7. **Other settings.** `net.handshakeSeconds` (5), `net.maxPendingHandshakes` (16).

## Alternatives considered

Each choice and its alternatives are in `docs/network/04-decisions.md` (D1–D36), with the RFC section
that specifies it.

## Consequences

- Reference 04, 06, 10, 13, 14, `game-api.md` and guides 12, 15, 27, 28 describe the new API.
- Tasks: SRV-01, SRV-02, SRV-05, SRV-13, SRV-14, ENG-22, ENG-38, ENG-39 gain the matching work; new
  tasks SRV-16 (reconnection), SRV-17 (replication settings and send-until-acknowledged), SRV-18 (fuzz
  tests), SRV-19 (Wireshark dissector), SRV-20 (network measurements under netem).
