# 23 · Testing your game

## 1. Pure functions: plain unit tests

Keep calculations out of systems so they can be tested without the engine:

```cpp
// Combat.hpp
int applyDamage(int hp, int damage, bool shielded);   // pure, testable anywhere

// in the system
hp.hp = applyDamage(hp.hp, dmg.amount, world.has<Shield>(e));
```

Test these with the project's test framework (chosen in the Tooling part).

## 2. Headless runs: the whole game without a window

The `null` plugins replace the real backends:

```bash
./r-type_client --platform null --audio null --ticks 1000      # runs 1000 steps, then quits
./r-type_server --ticks 600 --log debug
```

- `--platform null` / `--audio null` load the built-in Null Objects instead of plugins.
- `--ticks <n>` quits cleanly after `n` simulation steps; the exit code tells whether anything failed.
- Every system runs; nothing is drawn or heard. This catches crashes, schedule errors, script
  errors and bad scene files in CI, without a screen.

## 3. Scene and prefab files

Loading is all-or-nothing with `file:line` errors. A CI job that starts each scene headless for a
few ticks validates every YAML file:

```bash
for s in scenes/*.yaml; do ./r-type_server --scene "$s" --ticks 5 || exit 1; done
```

## 4. Network

Run a headless server and two headless clients on the same machine with `--ticks`, and check the
logs for `PlayerJoined`. Network conditions (latency, loss) are tested in the Network part.

## What to test first

1. Damage and death rules (pure functions).
2. Every scene and prefab loads (headless).
3. A full headless match of N ticks without errors.
