# 26 · Best practices

## Naming

| Thing | Convention | Example |
|---|---|---|
| component | noun, PascalCase | `Health`, `BydosAI`, `Shield` |
| typed tag | noun, empty struct | `Enemy`, `PlayerShot` |
| event | past tense, PascalCase | `PlayerDied`, `WaveCleared` |
| system name | `<game>.<verb_or_noun>` | `rtype.player_control`, `rtype.damage` |
| action | PascalCase verb | `MoveUp`, `Fire`, `ToggleForce` |
| CVar | `<game>_<name>` lower case | `rtype_difficulty` |
| prefab and scene files | snake_case | `prefabs/bydos_small.yaml` |

## Data and logic

- **One concern per component.** `Health` and `Shield`, not `Stats`.
- **One job per system.** "move", "shoot", "apply damage": easier to order, profile and disable.
- **Data in YAML, behaviour in C++ or Lua.** Numbers designers tune (speeds, HP, cooldowns) never live in C++.
- **C++ for rules that run every tick on many entities; Lua for unique behaviours** (a boss, a scripted wave).

## Events or direct code?

| Situation | Use |
|---|---|
| the sender should not know who reacts (death → sound, score, FX, network) | an event |
| one system owns the whole behaviour | direct code in that system |
| something must happen in the same step, in a precise order | system ordering (`after` / `before`), not an event |

## Safety

- Change structure (spawn, destroy, add, remove) only through `ctx.commands()` inside systems.
- Keep `Entity` values, never pointers or references to components.
- No `static` or global state; no file I/O, sleeping or blocking calls in systems.
- Randomness: a seeded `std::mt19937` owned by a system or the game state, used on the server.
  Never `std::rand`.
- Every gameplay decision on the server. The client shows and predicts.

## Performance

- Iterate with views; prefer the most selective component first in your mind, the engine handles the rest.
- Avoid `find` by name in loops; resolve once and keep the `Entity`.
- Give colliders tight layers and masks: skipped pairs cost nothing.
- Recycle nothing by hand: the ECS reuses slots already.
- Measure with the overlay (F2) and `systems` before optimising.

## Working as a team

- One owner per system file; new components and events go through review (they are a shared vocabulary).
- New engine-level needs become an ADR, not a hack in the game module.
