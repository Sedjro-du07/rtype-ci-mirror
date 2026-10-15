# 4 · Reflection and serialization

Status: **Locked**. ADRs 0005, 0014, 0018, 0027, 0036 (one `add<T>()`, type categories), 0037 (messages).
Folder: `engine/include/engine/reflection/`, `engine/src/reflection/`.

## Role

Teach the engine the types a game invents, so it can store them (ECS), read and write them (YAML),
show them (editor, console), send them (network) and expose them (scripts) without being compiled
against them. Reflection by explicit registration, no code generation.

---

## Type keys and categories — `reflection/TypeKey.hpp`

Cross-module identity never uses `typeid` or static counters (ADR 0005). Each type the engine must
know declares its name **and its category** once, with a macro placed right after the type, outside
any namespace (ADR 0036).

```cpp
enum class TypeCategory : uint8_t { Component, Event, Message, Struct, Enum };

template <class T> struct TypeName;               // specialised by the macros below:
                                                  //   value = "Health", key = fnv1a64("Health"), category

#define ENGINE_COMPONENT(Type)  /* data attached to entities: ECS, YAML, Lua, inspector, replication   */
#define ENGINE_EVENT(Type)      /* published on the EventBus, inside the process only                  */
#define ENGINE_MESSAGE(Type)    /* sent between server and clients (block 10)                          */
#define ENGINE_STRUCT(Type)     /* plain record used only as a field of another type (alone or in a list) */
#define ENGINE_ENUM(Type, ...)  /* enumeration + its value names, for YAML, Lua and the inspector       */

template <class T> concept HasTypeName = requires { TypeName<T>::key; };
template <class T> constexpr uint64_t typeKey() noexcept;   // TypeName<T>::key
// ComponentId (uint32_t, dense index assigned at registration) is declared in ecs/IStorage.hpp (ENG-17)
```

| Macro | Category | Becomes | Example |
|---|---|---|---|
| `ENGINE_COMPONENT(T)` | Component | storage in every scene, YAML, Lua, inspector, replication if `Replicated` | `Health`, `Position` |
| `ENGINE_EVENT(T)` | Event | `EventBus` publish / subscribe, Lua `publish` | `BossDefeated`, `Collision` |
| `ENGINE_MESSAGE(T)` | Message | `NetworkSession::send` / `broadcast` / `on<T>`, a network id | `Chat`, `Ready`, `RoomList` |
| `ENGINE_STRUCT(T)` | Struct | a field kind usable inside the others | `RoomInfo`, `ShotPattern` |
| `ENGINE_ENUM(T, values…)` | Enum | names ↔ values; integer on the wire | `EnemyKind` |

Rules:

- The macro accepts a qualified name (`ENGINE_COMPONENT(rtype::Health)`); the registered name is the
  last part (`Health`), so files stay readable.
- A second registration with the same name, or two names with the same hash, is a boot error
  (`DuplicateName`, exit code 3).

Module macros (`ENGINE_GAME_MODULE`, `ENGINE_PLUGIN`, `ENGINE_EXPORT`) are in block 2.

## `enum class FieldKind` and `struct FieldInfo` — `reflection/TypeInfo.hpp`

```cpp
enum class FieldKind : uint8_t {
    Bool, Int32, UInt32, Int64, Float, Double, String,
    Vec2, Rect, Color, Entity,                          // Entity: written as the target's Uuid in files
    Texture, Sound, Music, Font, Script,                // asset handles: written as paths in files
    Enum,                                               // a type registered with ENGINE_ENUM
    List,                                               // std::vector<T> of any kind above: count, then elements
    Struct,                                             // a type registered with ENGINE_STRUCT
    Custom                                              // yaml() / binary() supplied by the author
};

struct FieldInfo {
    std::string         name;          // YAML key and inspector label
    FieldKind           kind;
    size_t              offset;        // offsetof within the component
    const EnumInfo*     enumInfo = nullptr;   // when kind == Enum
    FieldKind           element{};            // when kind == List: kind of the elements
    uint64_t            elementType = 0;      // when kind == List or Struct: key of the struct type
    uint32_t            maxLength = 0;        // String and List: limit checked before any allocation
    float               quantMin = 0, quantMax = 0;   // Float and Vec2: quantised on the wire when min < max
                                              // (0 = default: 256 characters, 64 elements)
    std::string         tooltip;       // optional, shown in the inspector
};
```

## `enum TypeFlags` and `struct TypeInfo` — `reflection/TypeInfo.hpp`

```cpp
enum TypeFlags : uint32_t {
    None         = 0,
    Replicated   = 1 << 0,   // sent by the replication system (block 10)
    EditorHidden = 1 << 1,   // not shown in the inspector
    Tag          = 1 << 2,   // empty struct used as a typed tag
    Transient    = 1 << 3,   // never saved to scene files (runtime-only state)
};                           // components only

enum class Transport : uint8_t { Unreliable, Reliable, Stream };   // messages: UDP, UDP + acks, TCP (block 10)

struct TypeInfo {
    ComponentId               id;
    uint64_t                  key;          // fnv1a64(name)
    std::string               name;
    TypeCategory              category;
    size_t                    size, align;
    uint32_t                  flags;
    std::vector<FieldInfo>    fields;
    void (*construct)(void* dst);                         // default-construct
    void (*destroy)(void* obj) noexcept;
    void (*copy)(void* dst, const void* src);             // copy-construct
    std::unique_ptr<IStorage> (*makeStorage)(ComponentId);// creates SparseSet<T>
    Result<void> (*fromYaml)(void* obj, const YamlNode&, LoadContext&); // generated from fields, or custom
    void         (*toYaml)(const void* obj, YamlWriter&, SaveContext&);
    void         (*encode)(const void* obj, ByteWriter&, const WireContext&); // generated from fields, or custom
    Result<void> (*decode)(void* obj, ByteReader&, const WireContext&);
    Transport                 transport = Transport::Reliable; // messages only (block 10)
};
```

`YamlNode` / `YamlWriter` are thin engine wrappers so that yaml-cpp stays out of game headers.
`WireContext` lets the codecs translate `Entity` fields: it holds two functions supplied by the
network block, `Entity → NetworkId` (0 if the entity is not replicated, with a warning) and
`NetworkId → Entity` (`Entity::null()` if unknown). `LoadContext` / `SaveContext` do the same with `Uuid`.

## Builders — `reflection/TypeBuilder.hpp`

`TypeRegistry::add<T>()` returns the builder of the category of `T` (ADR 0036). The builders that
describe fields inherit `field()` from `FieldsBuilder<T>`; *deducing this* (ADR 0017) makes `field()`
return the concrete builder so options can follow it in one chain.

```cpp
template <class T>
class FieldsBuilder {
public:
    // Deduces FieldKind from the member type (bool, ints, float, double, std::string, Vec2, Rect, Color,
    // Entity, asset handles, ENGINE_ENUM, ENGINE_STRUCT, std::vector of those). Unsupported → compile error.
    template <class Self, class M>
    Self& field(this Self& self, std::string_view name, M T::*member, std::string_view tooltip = {});
    template <class Self>
    Self& maxLength(this Self& self, uint32_t n);   // limit of the last String or List field declared
    template <class Self>
    Self& quantize(this Self& self, float min, float max);   // last Float or Vec2 field travels as u16 (RFC §5, §9.6)
};

template <class T> class ComponentBuilder : public FieldsBuilder<T> {
public:
    ComponentBuilder& flags(uint32_t f);                    // Replicated | Transient | EditorHidden | Tag
    ComponentBuilder& yaml(FromYamlFn read, ToYamlFn write);// custom YAML instead of field-by-field
    ComponentBuilder& binary(EncodeFn encode, DecodeFn decode); // custom wire format (e.g. quantised)
};

template <class T> class MessageBuilder : public FieldsBuilder<T> {
public:
    MessageBuilder& unreliable();   // UDP, may be lost (block 10)
    MessageBuilder& reliable();     // UDP with acknowledgements: must arrive, in game (default)
    MessageBuilder& stream();       // TCP: out of game (lobby, room lists, text chat)
    MessageBuilder& binary(EncodeFn encode, DecodeFn decode);
};

template <class T> class EventBuilder  : public FieldsBuilder<T> {};   // fields for Lua and the console
template <class T> class StructBuilder : public FieldsBuilder<T> {};   // fields only
template <class T> class EnumBuilder   {};                              // nothing: the macro said it all
```

Example:

```cpp
void RType::registerTypes(engine::TypeRegistry& types) {
    types.add<Health>().field("hp", &Health::hp).field("maxHp", &Health::maxHp).flags(engine::Replicated);
    types.add<BossDefeated>().field("score", &BossDefeated::score);
    types.add<RoomInfo>().field("name", &RoomInfo::name).maxLength(32).field("players", &RoomInfo::players);
    types.add<RoomList>().field("rooms", &RoomList::rooms).maxLength(32).stream();
    types.add<EnemyKind>();
}
```

## `class TypeRegistry` — `reflection/TypeRegistry.hpp`

```cpp
class TypeRegistry {
public:
    template <class T> auto add();                          // the only registration method: returns the
                                                            // builder of TypeName<T>::category (static_assert
                                                            // if T has no type macro)

    template <class T> ComponentId id() const;              // hash lookup by typeKey<T>(), O(1)
    const TypeInfo* find(std::string_view name) const;      // nullptr if unknown (loader suggests a close name)
    const TypeInfo* findByKey(uint64_t key) const;
    const TypeInfo& info(ComponentId id) const;
    std::span<const TypeInfo> all() const;                  // inspector, console "types"
    std::span<const TypeInfo* const> messages() const;      // in registration order: source of message ids
    uint64_t formatFingerprint() const;                     // hash of every message and Replicated component
                                                            // (names, fields, kinds, order): compared at handshake
    std::string closestName(std::string_view unknown) const;// "Helth" → "Health"
    void clearGameTypes();                                  // before unloading a game module (ADR 0014)
};
```

Engine types are registered first (block 13), then `IGameModule::registerTypes`.

## Generic codecs

`TypeInfo::encode` / `decode` (and the YAML pair) are written **once** in the engine and walk the
field list of any type: no code per component or message. Per field kind:

| Kind | In memory | In a file | On the wire |
|---|---|---|---|
| `Entity` | `Entity` | the target's `Uuid` | its `NetworkId`; unknown on arrival → `Entity::null()` (block 10) |
| `List` | `std::vector<T>` | YAML sequence | count, then elements; count > limit or bytes missing → rejected before allocating |
| `Struct` | the struct | YAML map | its fields in order |
| `Enum` | the enum | the value name | an integer |
| asset handles | handle | path | path |

A decode failure (missing bytes, limit exceeded, unknown enum value) rejects the whole value: no
partial object reaches game code. The binary codec is the private class `Codec` (`engine/src/reflection/Codec.hpp`,
task SRV-04); the YAML codec lives in `Yaml.cpp` (ENG-23).

## Byte streams — `reflection/ByteStream.hpp`

```cpp
class ByteWriter {                     // appends to a caller-owned buffer, never allocates beyond a limit
public:
    explicit ByteWriter(std::vector<std::byte>& out, size_t maxBytes);
    bool write(uint8_t); bool write(uint16_t); bool write(uint32_t); bool write(uint64_t);
    bool write(int32_t); bool write(float); bool write(std::string_view);   // length-prefixed
    bool writeBits(uint32_t value, uint8_t bitCount);                        // bit packing (Track 2)
    size_t size() const; bool overflowed() const;
};
class ByteReader {                     // bounds-checked: every read fails cleanly past the end
public:
    explicit ByteReader(std::span<const std::byte> in);
    std::optional<uint8_t> u8(); std::optional<uint16_t> u16(); std::optional<uint32_t> u32();
    std::optional<uint64_t> u64(); std::optional<int32_t> i32(); std::optional<float> f32();
    std::optional<std::string> string(size_t maxLength);
    std::optional<uint32_t> bits(uint8_t bitCount);
    size_t remaining() const; bool failed() const;
};
```

Byte order is big-endian, floats as their IEEE 754 bit pattern (network step 4, D1); the encoding of
each wire field is fixed by the protocol RFC (Network part).
These classes guarantee bounds and limits so that a malformed packet can never read or allocate
out of range.
