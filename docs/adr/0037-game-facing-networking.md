# ADR 0037: The game sees players and typed messages, never sockets or bytes

- Status: Accepted
- Date: 2026-10-14
- Deciders: the R-Type team
- Discussion id: SRV-1 to SRV-4 (Server part, round 1)
- Amends: [ADR 0033](0033-engine-networking.md)

## Context

ADR 0011 and 0033 put networking in the engine, but the game still called `host()` / `connect()`,
handled `ConnectionId`s and wrote message payloads byte by byte with an id it chose. The server is
part of the engine; the game must only state its rules (who spawns when a player arrives, which
messages exist) without managing sockets or bytes.

## Decision

1. **The game opens the server explicitly.** `NetworkSession` offers `startServer(port)` (port from
   `--port` / config when omitted), `stopServer()`, `connectTo(host, port, name)` and `disconnect()`.
   Nothing opens automatically: tests run without a port, and a lobby or a "host a game" menu decides
   when to open.
2. **Players, not connections.** The game sees `PlayerId` (0 = none), events `PlayerJoined`,
   `PlayerLeft{reason}`, `JoinedServer{me}`, `LeftServer{reason}`, and `players()`, `localPlayer()`,
   `name(player)`, `kick`, `ping`. One connection is one player in Part 1. Each player has a name
   given to `connectTo`. `ConnectionId` stays inside the engine.
3. **Typed messages.** A message is a struct declared with `ENGINE_MESSAGE` and registered with
   `types.add<T>()` (ADR 0036). The engine serializes it from its fields through reflection, with
   default limits (text 256, list 64) overridden by `.maxLength(n)`. `Entity` fields travel as
   `NetworkId` and come back as `Entity::null()` when unknown. Games send with `send(player, msg)` /
   `broadcast(msg, except)` and receive with `on<T>(fn)`, which returns a `Subscription`. Callbacks
   run on the loop thread during `NetworkSession::poll()`, before `FixedUpdate`; an exception in a
   callback is caught, logged, and the message is ignored. Raw byte sending is engine-internal.
4. **Three transports, chosen on the message.** `.unreliable()` (UDP), `.reliable()` (UDP with
   acknowledgements, the default, for in-game messages as the subject requires UDP in game) and
   `.stream()` (TCP, for out-of-game traffic: lobby, room lists, text chat). The engine links a
   player's TCP and UDP connections to one `PlayerId`.
5. **Format fingerprint.** The engine hashes every message and `Replicated` component (names,
   fields, kinds, order) and compares client and server at the handshake; a mismatch refuses the
   connection with a clear message.

Byte layout, message ids, the reliability scheme, how TCP and UDP are linked and how the name and the
fingerprint travel are defined by the protocol RFC (Network part).

## Alternatives considered

- **Server opened automatically in the server role:** no control for lobbies, tests open real ports,
  no "host a game" from a client.
- **Copying structs as raw bytes:** padding, byte order and pointers (`std::string`) differ; trusts
  untrusted input.
- **Hand-written serialize/deserialize per message (Quake, Source):** two functions to keep in sync
  per message; bytes in game code.
- **Code generators (Protobuf, FlatBuffers):** a build dependency, and we lose control of the
  protocol we must design.
- **Messages read by systems (`received<T>()`):** fits ECS but reads less directly for rare
  messages; can be added later.
- **Two transports only:** in-game messages that must arrive cannot use TCP (subject), and lobby or
  chat traffic gains nothing from reliable UDP.

## Consequences

- Game network code is declarative: flags on components, messages as structs, callbacks for rules.
- TCP is no longer optional in the transport plugin: `network_asio` implements it from Part 1.
- `NetOwner` holds a `PlayerId`. The engine events `ClientConnected`, `ClientDisconnected`,
  `ConnectedToServer`, `DisconnectedFromServer` are replaced by the player events above.
- Voice chat would use UDP but needs microphone capture and a codec; it is out of scope for now.
