# 05 · Entities, views and commands

## Query

```cpp
auto& world = ctx.world();

for (auto [e, pos, vel] : world.view<engine::Position, engine::Velocity>()) { /* … */ }

for (auto [e, hp] : world.view<Health>(engine::exclude<Invincible>)) { /* … */ }

world.view<Enemy>().each([&](engine::Entity e) { ++count; });
```

Read and write component values freely inside the loop.

## Single entity

```cpp
if (auto* hp = world.tryGet<Health>(e)) hp->hp -= 1;   // absent → nullptr
bool armed = world.has<Gun>(e);
auto& pos  = world.get<engine::Position>(e);           // asserts present
world.replace<Health>(e, Health{5, 5});                 // fires onReplace hooks
bool stillThere = world.alive(e);                       // false after destroy, even if the slot is reused
```

## Change structure: always through `Commands`

```cpp
auto& cmd = ctx.commands();
engine::Entity shot = cmd.spawn("prefabs/missile.yaml", {pos.x + 20, pos.y});
cmd.add(shot, engine::Lifetime{2.f});
cmd.remove<Shield>(player);
cmd.destroy(enemy);                // children are destroyed too
```

Commands are applied at the end of the stage, in the order recorded. `spawn()` returns the entity
immediately so you can add more commands to it.

Direct `world.emplace` / `world.remove` / `world.destroy` exist for code that runs outside system
iteration (`onStart`, tools, tests).

## Identity

```cpp
engine::Entity boss = world.find("Dobkeratops");          // by Name
engine::Entity e    = world.find(uuid);                    // by Uuid (scene files)
auto flying         = world.withTag("flying");             // text tags from YAML / scripts
```

## Hierarchy

```cpp
world.setParent(force, ship, {26.f, 0.f});   // the Force follows the ship with this offset
engine::Entity owner = world.parent(force);
for (engine::Entity child : world.children(ship)) { /* … */ }
world.setParent(force, engine::Entity::null()); // detach
```

`engine.hierarchy` moves children after `engine.movement`. Destroying a parent destroys its children.

## Hooks

```cpp
// in onStart; keep the Subscription alive as long as you need it
subs_.push_back(ctx.world().hooks<Health>().onDestroy.connect(
    [](engine::Registry& r, engine::Entity e) { /* drop score popup */ }));
```

Hooks fire when the change is applied, never in the middle of a view.

## Rules

- Keep `Entity` values, never pointers to components.
- A destroyed entity's handle stays invalid forever, even if its slot is reused.
