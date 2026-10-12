# 20 · Hierarchy and identity

## Identity components

| Component | Purpose | When to give it |
|---|---|---|
| `Name` | a readable name: logs, console, `world.find("Boss")` | entities you look up or debug (boss, game state, player) |
| `Uuid` | stable identity in files | every entity written in a scene file (the loader assigns it) |
| `Tags` | free text labels from YAML and scripts | labels designers add without C++ |
| typed tag (`struct Enemy {};`) | fast C++ queries | categories your systems iterate |

All are optional: 2 000 missiles carry none of them.

### Typed tags or text tags?

```cpp
for (auto [e, pos] : world.view<engine::Position>(engine::exclude<Enemy>)) { … }  // typed: fast, compile-checked
auto flyers = world.withTag("flying");                                           // text: designers' labels
```

Use typed tags for anything code depends on; text tags for designer-level grouping and scripts.

## Parent and children

`Parent { Entity value; Vec2 offset; }` makes an entity follow another. `engine.hierarchy` sets the
child's `Position` to the parent's `Position` + `offset` every step, after movement.

```cpp
// inside a system: structural change, so through Commands
ctx.commands().setParent(force, ship, {26.f, 0.f});        // attach to the front of the ship

// move it to the back: a value change, immediate and safe
ctx.world().get<engine::Parent>(force).offset = {-26.f, 0.f};

// detach and let it fly on its own
ctx.commands().setParent(force, engine::Entity::null());
ctx.commands().set(force, engine::Velocity{300.f, 0.f});
```

In YAML, children are declared inside the prefab:

```yaml
children:
  - prefab: prefabs/force.yaml
    Parent: { offset: { x: 26, y: 0 } }
```

### Rules

- `world.setParent(...)` is immediate: use it outside view loops (`onStart`, tools). Inside systems use `ctx.commands().setParent(...)`.
- **Destroying a parent destroys its children.** Detach first if a child must survive.
- A child with `Parent` should not have its own `Velocity` while attached: the hierarchy overwrites its position.
- Hierarchies are shallow by design (ship → Force, boss → turrets). Deep trees cost one lookup per level.
- Rotation and scale are not inherited, only position.

## Finding entities

| Need | Call |
|---|---|
| one known entity | `world.find("Dobkeratops")` |
| the entity of a scene uuid | `world.find(uuid)` |
| all entities of a kind | `world.view<Enemy>()` |
| all entities with a text tag | `world.withTag("flying")` |
| the owner of a child | `world.parent(e)` |
| the children of an entity | `world.children(e)` |

`find` by name is linear; call it in `onStart` or rarely, and keep the `Entity` you get.
