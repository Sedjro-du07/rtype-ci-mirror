# ADR 0030: Input is read as named actions with accessibility options

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 7

## Context

Game code must not depend on keys or backends; the subject grades motor accessibility.

## Decision

Plugins produce `RawInputEvent`s (physical keys, mouse, gamepad). `InputMap` loads `input.yaml` (several bindings per action, alternate one-handed set, options), supports rebinding with conflict detection and saving, and publishes an `ActionState` per tick with latched edges. Options: hold-to-toggle, auto-repeat, deadzone, one-handed set. `ActionState` converts to and from a bit mask for the network.

## Alternatives considered

- **Keys read directly in systems:** no rebinding, backend-dependent code.

## Consequences

- Accessibility features apply to every game and every backend.
- Actions are hashes of names, so games define them freely.
