# ADR 0041: Network optimizations: the engine gives the means, the game team optimizes

- Status: Accepted (amends [ADR 0033](0033-engine-networking.md))
- Date: 2026-10-14
- Deciders: the R-Type team
- Discussion id: NET-1 (Network part, step 1)

## Context

ADR 0033 planned prediction, reconciliation, delta snapshots and compression as engine work "on top
of" replication, with client interpolation built in and no game code needed. The engine is meant to
be used as a product: a team makes an online game with it **without access to the engine's code**,
and the person in charge of that game's server owns its network optimizations, as they own its rules.

That person can only optimize what the engine exposes. The engine also keeps the bytes: the protocol
is defined by the engine and a game never writes bytes (ADR 0037, 0040).

## Decision

1. **The engine does only what is impossible from outside:** the protocol and its bytes, sequence
   numbers, acknowledgements, the tick carried by each snapshot, input sequence numbers, the last
   input processed per player, the history of unconfirmed inputs.
2. **The engine exposes settings** for what a game can optimize without touching bytes (for example
   rates, field precision, who receives what, priorities). Which settings: Network part, steps 2 and 4.
3. **The engine offers extension points:** the game team writes its own prediction and reconciliation,
   and its own interpolation if it wants one, as game systems (`shared/` and `client/`, ADR 0040)
   using the data the engine provides.
4. **Engine defaults can be turned off:** the built-in interpolation (`engine.net.interpolate`) stays
   as a default; a game disables it (`SystemBuilder::disabled`, `Scheduler::setEnabled`) to use its
   own. Without access to the code, a default that can be turned off is better than nothing.
5. **Bandwidth targets are the game team's**, reached through the engine's settings. The engine must
   offer enough settings for a team to stay playable on a constrained link; the subject's test
   condition of 5 KB/s is the reference case.

The same split as rooms (ADR 0038, option C): mechanism in the engine, policy in the game.

## Alternatives considered

- **Everything in the engine (ADR 0033 as written):** a second game gets everything for free, but a
  team without the engine's code cannot change how its game feels or what it costs on the wire.
- **Everything in the game:** a game would have to write bytes, sequence numbers and acks itself,
  which breaks ADR 0037 (the game never writes bytes) and makes every game rewrite the protocol.

## Consequences

- The protocol RFC carries from v1.0 the fields prediction needs (input sequence numbers, last input
  processed, snapshot tick), even though prediction is written later by a game team; adding them
  later would break the format.
- The engine's settings and extension points for network optimization become part of the game-facing
  API and are designed in the Network part (steps 2 and 4).
- Delta snapshots and quantization stay engine mechanisms (they touch bytes); whether and how much a
  game uses them is a setting.
- Industry reference: Unreal (frequency, relevancy, priority, quantized types set by the game), Unity
  Netcode (built-in interpolation that can be switched off, prediction left to the developer).
