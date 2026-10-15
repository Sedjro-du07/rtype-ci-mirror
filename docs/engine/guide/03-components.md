# 03 · Components

A component is a plain struct: data only, no logic, no pointers to other components.

## Define

```cpp
// src/Components.hpp
#include <engine/reflection/TypeKey.hpp>

enum class Pattern { Straight, Sine, Dive };
ENGINE_ENUM(Pattern, Straight, Sine, Dive)

struct Health { int hp = 3; int maxHp = 3; };
ENGINE_COMPONENT(Health)

struct BydosAI { float shootCooldown = 1.f; float timer = 0.f; Pattern pattern = Pattern::Straight; };
ENGINE_COMPONENT(BydosAI)

struct Enemy {};                       // empty struct = typed tag
ENGINE_COMPONENT(Enemy)
```

`ENGINE_COMPONENT` gives the type its name **and says it is a component**. Never rely on `typeid`;
the name is what scenes, scripts, the editor and the network use. Put the macro right after the
type, outside any namespace; a qualified name works (`ENGINE_COMPONENT(rtype::Health)` registers
`Health`).

## The five kinds of types

The macro decides what the engine does with the type (ADR 0036):

| Macro | The type is | Used with |
|---|---|---|
| `ENGINE_COMPONENT(T)` | data attached to an entity | `world()`, views, YAML, Lua, replication |
| `ENGINE_EVENT(T)` | a notification inside this process | `events().publish` / `subscribe` (page 06) |
| `ENGINE_MESSAGE(T)` | a message between server and clients | `network().send` / `on<T>` (page 12) |
| `ENGINE_STRUCT(T)` | a small record used inside another type | as a field, alone or in a `std::vector` |
| `ENGINE_ENUM(T, values…)` | an enumeration readable by name | as a field (`pattern: Sine` in YAML) |

All of them are registered the same way, with `types.add<T>()`. The engine returns the right
"form" for the kind, and only offers the options that make sense for it: `flags` exists on a
component, `reliable` on a message. A wrong option does not compile.

## Register

```cpp
void MyGameModule::registerTypes(engine::TypeRegistry& t) {
    t.add<Pattern>();
    t.add<Health>().field("hp", &Health::hp).field("maxHp", &Health::maxHp)
                   .flags(engine::Replicated);
    t.add<BydosAI>().field("shootCooldown", &BydosAI::shootCooldown)
                    .field("pattern", &BydosAI::pattern);       // timer not listed: runtime-only
    t.add<Enemy>().flags(engine::Tag);
}
```

Read the chain as filling a form: `add<Health>()` opens the form for `Health`, each `field(name,
&Health::member)` adds a line (the name used in files and scripts, and where the member is in the
struct), `flags(...)` ticks options. Only listed fields are read from YAML, saved, shown in the
editor and sent over the network.

## Field types

`bool`, `int32_t`, `uint32_t`, `int64_t`, `float`, `double`, `std::string`, `Vec2`, `Rect`,
`Color`, `Entity` (saved as the target's uuid), `TextureHandle`, `SoundHandle`, `MusicHandle`,
`FontHandle`, `ScriptHandle` (saved as paths), enums declared with `ENGINE_ENUM`, structs declared
with `ENGINE_STRUCT`, and `std::vector` of any of these. Text and lists are limited (256 characters,
64 elements by default); change the limit of the field just declared with `.maxLength(n)`.
Anything else needs a custom codec: `.yaml(read, write)` and, if replicated, `.binary(encode, decode)`.

```cpp
struct ShotPattern { float angle; float speed; };
ENGINE_STRUCT(ShotPattern)
struct Weapon { std::vector<ShotPattern> shots; float cooldown; };
ENGINE_COMPONENT(Weapon)

t.add<ShotPattern>().field("angle", &ShotPattern::angle).field("speed", &ShotPattern::speed);
t.add<Weapon>().field("shots", &Weapon::shots).maxLength(8).field("cooldown", &Weapon::cooldown);
```

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
