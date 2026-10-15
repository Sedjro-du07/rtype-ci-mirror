# 11 · Scripting with Lua

Behaviour that designers change often (waves, boss phases, special enemies) goes in Lua. Data stays
in YAML (ADR 0018).

## Attach a script

```yaml
# prefabs/dobkeratops.yaml
Sprite: { texture: assets/boss1.png, layer: 25 }
Health: { hp: 500, maxHp: 500 }
Script: { file: scripts/dobkeratops.lua }
```

## Hooks

```lua
-- scripts/dobkeratops.lua
local phase = 1

function on_spawn(self)
    self:set("Velocity", { dx = -20, dy = 0 })
end

function on_update(self, dt)
    local hp = self:get("Health").hp
    if phase == 1 and hp < 250 then
        phase = 2
        local p = self:get("Position")
        spawn("prefabs/bydos_small.yaml", p.x, p.y + 40)
        play("assets/roar.wav", { caption = "[boss roars]" })
    end
end

function on_collision(self, other)
    if other:has_tag("player_shot") then publish("BossHit", { boss = self }) end
end

function on_destroy(self)
    publish("BossDefeated", {})
end
```

All hooks are optional. `on_update` runs every fixed step.

## API

| Call | Does |
|---|---|
| `self:get(name)` | copy of a component as a table (`nil` if absent) |
| `self:set(name, table)` | adds or replaces a component (applied at the end of the stage) |
| `self:has(name)`, `self:remove(name)` | |
| `self:has_tag(tag)`, `self:id()` | |
| `spawn(prefab, x, y)` | returns the new entity |
| `destroy(entity)` | |
| `find(name)` | entity by `Name` |
| `play(sound, params)` | `params`: `volume`, `pan`, `caption` |
| `publish(event, table)` | event registered by the game, fields by name |
| `time()` | `{ dt, tick, total }` |
| `cvar(name)` | read a console variable |
| `log(text)` | |

Components and events are accessed by the names given in `registerTypes`: no binding code per type.

## Sandbox and errors

- No file, OS or network access; no loading other code.
- A script that runs too long in one call is stopped.
- An error prints `scripts/dobkeratops.lua:12: attempt to index a nil value`, disables that
  entity's script, and the game continues.

## Hot reload

Save the file while the game runs with `asset_hot_reload 1`: the script is reloaded, entities keep
their components. Local variables of the script restart.
