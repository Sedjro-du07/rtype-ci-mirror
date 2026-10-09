# 03 · Components

A component is a plain struct: data only, no logic, no pointers to other components.

## Define

```cpp
// src/Components.hpp
#include <engine/reflection/TypeKey.hpp>

struct Health { int hp = 3; int maxHp = 3; };
ENGINE_COMPONENT(Health)

struct BydosAI { float shootCooldown = 1.f; float timer = 0.f; Pattern pattern = Pattern::Straight; };
ENGINE_COMPONENT(BydosAI)

struct Enemy {};                       // empty struct = typed tag
ENGINE_COMPONENT(Enemy)

enum class Pattern { Straight, Sine, Dive };
ENGINE_ENUM(Pattern, Straight, Sine, Dive)
```

`ENGINE_COMPONENT` gives the type its name. Never rely on `typeid`; the name is what scenes,
scripts, the editor and the network use.

## Register

```cpp
void MyGameModule::registerTypes(engine::TypeRegistry& t) {
    t.addEnum<Pattern>();
    t.add<Health>().field("hp", &Health::hp).field("maxHp", &Health::maxHp)
                   .flags(engine::Replicated);
    t.add<BydosAI>().field("shootCooldown", &BydosAI::shootCooldown)
                    .field("pattern", &BydosAI::pattern);       // timer not listed: runtime-only
    t.add<Enemy>().flags(engine::Tag);
}
```

Only listed fields are read from YAML, saved, shown in the editor and sent over the network.

## Field types

`bool`, `int32_t`, `uint32_t`, `int64_t`, `float`, `double`, `std::string`, `Vec2`, `Rect`,
`Color`, `Entity` (saved as the target's uuid), `TextureHandle`, `SoundHandle`, `MusicHandle`,
`FontHandle`, `ScriptHandle` (saved as paths), enums declared with `ENGINE_ENUM`. Anything else
needs a custom codec: `.yaml(read, write)` and, if replicated, `.binary(encode, decode)`.

## Flags

| Flag | Effect |
|---|---|
| `Replicated` | sent from server to clients by the replication systems (12) |
| `Transient` | never saved to scene files |
| `EditorHidden` | not shown in the inspector |
| `Tag` | empty struct used only for queries |

## Rules

- A component has no virtual functions and no logic. Helpers that only compute from its own fields are fine.
- Never store a pointer or reference to another component. Store the `Entity`.
- Keep components small and focused: `Health` and `Shield`, not `Stats` with twenty fields.
- Names are unique across the engine and the game: you cannot define your own `Position`.

## Built-in components

`Position`, `PreviousPosition`, `Velocity`, `Sprite`, `Animation`, `Text`, `Collider`, `Lifetime`,
`Parallax`, `Name`, `Uuid`, `Tags`, `Parent`, `Children`, `Script`, `NetworkId`, `NetPrefab`,
`NetOwner`, `RemoteInput`, `NetInterpolation`. Fields: reference page 13.
