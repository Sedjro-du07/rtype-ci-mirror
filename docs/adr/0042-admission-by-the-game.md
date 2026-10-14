# ADR 0042: Admission: the engine checks it can decode, the game decides who joins

- Status: Accepted (amends [ADR 0037](0037-game-facing-networking.md))
- Date: 2026-10-14
- Deciders: the R-Type team
- Discussion id: NET-2 (Network part, step 2, group 1)

## Context

The handshake (ADR 0037, task SRV-02) refuses a client for `ServerFull`, `FormatMismatch` or an invalid
name, all decided by the engine. The game only hears of a player once accepted (`PlayerJoined`).
Under ADR 0041 (the engine is used without its code; mechanism in the engine, policy in the game),
who may join a game server is a rule of the game, not of the engine.

## Decision

1. **The engine keeps what is an impossibility, not a rule**, automatically and without a way to turn
   it off: the protocol version (otherwise the server cannot even read the Hello) and the format
   fingerprint (otherwise decoding gives wrong values).
2. **The game decides admission** through an extension point: the Hello may carry data of the game,
   declared as a typed message (never raw bytes); the game server receives an admission request (name
   and that data) and accepts, or refuses with its own reason; the client receives that reason to show
   it. Typical rules: game or content version, right game, name rules beyond the length limit,
   password, allow list, bans.
3. **Order:** engine checks first; the game is asked only about a client the engine can decode.
4. The form of the extension point (event or callback, message, reason) is decided in step 4 and
   written in the protocol RFC.

## Alternatives considered

- **Engine decides everything (ADR 0037 as written):** a team without the engine's code cannot add a
  password, a ban list or a minimum game version.
- **Game decides everything, including format:** accepting a client the engine cannot decode gives a
  corrupted game, not a tolerant one.

## Consequences

- Industry: Unreal (`GameMode::PreLogin` with an error message, after the engine's network version
  check), Unity Netcode (`ConnectionApprovalCallback` with client data, after the `ProtocolVersion`
  hash check), Godot (`SceneMultiplayer.auth_callback`).
- The handshake gains an admission stage between the engine's checks and the challenge; SRV-02 follows
  the RFC for it.
