# 18 · Collisions

The engine **detects** overlaps and tells you; your game decides what they mean (damage, pickup,
death). There is no physical response in Part 1: nothing is pushed, bounced or blocked by the
engine (page 27).

## Give an entity a collider

```yaml
Collider:
  size:   { x: 30, y: 14 }      # box size in world units
  offset: { x: 0, y: 0 }        # box center relative to Position
  layer:  1                     # bits: what this entity IS
  mask:   6                     # bits: what it can collide WITH
  trigger: false
```

The box is axis-aligned (AABB) and centered on `Position + offset`.

## Layers and masks

`layer` and `mask` are bit sets. Two colliders `a` and `b` produce a collision only if
`(a.layer & b.mask) != 0` **and** `(b.layer & a.mask) != 0`. Both sides must agree.

Suggested R-Type layout (one bit per kind):

| Bit | Value | Kind | Mask (collides with) |
|---|---|---|---|
| 0 | 1 | player | enemy (2) + enemy shot (4) + wall (16) = 22 |
| 1 | 2 | enemy | player (1) + player shot (8) = 9 |
| 2 | 4 | enemy shot | player (1) + wall (16) = 17 |
| 3 | 8 | player shot | enemy (2) + wall (16) = 18 |
| 4 | 16 | wall / obstacle | player, enemy shot, player shot = 13 |

Enemies never collide with each other, shots never collide with shots: those pairs are skipped
before any test, which also keeps collisions cheap.

## React

`engine.collision` runs in `FixedUpdate` after movement and publishes one `engine::Collision{a, b}`
per overlapping pair per tick (every tick while they overlap).

```cpp
class DamageSystem {
public:
    explicit DamageSystem(engine::EventBus& bus)
        : sub_(bus.subscribe<engine::Collision>([this](const engine::Collision& c) { pending_.push_back(c); })) {}

    void operator()(engine::Context& ctx) {            // server, after engine.collision
        auto& w = ctx.world();
        for (auto [a, b] : pending_) {
            if (!w.alive(a) || !w.alive(b)) continue;
            hit(ctx, a, b); hit(ctx, b, a);
        }
        pending_.clear();
    }
private:
    void hit(engine::Context& ctx, engine::Entity target, engine::Entity source) {
        auto* hp = ctx.world().tryGet<Health>(target);
        auto* dmg = ctx.world().tryGet<Damage>(source);
        if (!hp || !dmg) return;
        hp->hp -= dmg->amount;
        if (ctx.world().has<Projectile>(source)) ctx.commands().destroy(source);
    }
    engine::Subscription sub_;
    std::vector<engine::Collision> pending_;
};
```

Register it with `.after("engine.collision").roles(engine::ServerRole)`. Damage, deaths and pickups
are decided by the server only.

**Once per pair, not every tick:** if a contact should count once (pickup, checkpoint), remove the
collider or add a marker component when you handle it.

## Triggers

`trigger: true` marks a sensor (pickup, zone, checkpoint). The engine reports it exactly like any
collider; the flag is there so game rules can tell sensors from bodies with
`world.get<engine::Collider>(e).trigger`. It will also exclude the collider from physical response
when that arrives (Part 2).

## Walls and blocking

Since the engine does not push entities apart, a game that needs blocking (the player against stage
walls) does it in a system after `engine.collision`: on `Collision` with a wall, restore the
player's `PreviousPosition` into `Position`, or clamp it to the wall's box.

## Debug and performance

- `set r_debug_colliders 1` draws every box.
- The broad phase is a uniform grid; `phys_grid_cell` (default 64) is the cell size. Set it to about
  twice the size of your most common collider.
- Colliders without `Velocity` (walls) cost almost nothing.
