# ENG-22 · TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **04 Reflection** · status and owner live on the board.

## Goal

Teach the engine the types a game invents: names, kinds, fields, options (ADR 0027, 0036).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/reflection/TypeKey.hpp`
- `include/engine/reflection/TypeInfo.hpp`
- `include/engine/reflection/TypeBuilder.hpp`
- `include/engine/reflection/TypeRegistry.hpp`
- `src/reflection/TypeRegistry.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/reflection/TypeKey.hpp`

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
using ComponentId = uint32_t;                                // dense index assigned at registration
```

### `include/engine/reflection/TypeInfo.hpp`

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
                                              // (0 = default: 256 characters, 64 elements)
    std::string         tooltip;       // optional, shown in the inspector
};

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

### `include/engine/reflection/TypeBuilder.hpp`

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

void RType::registerTypes(engine::TypeRegistry& types) {
    types.add<Health>().field("hp", &Health::hp).field("maxHp", &Health::maxHp).flags(engine::Replicated);
    types.add<BossDefeated>().field("score", &BossDefeated::score);
    types.add<RoomInfo>().field("name", &RoomInfo::name).maxLength(32).field("players", &RoomInfo::players);
    types.add<RoomList>().field("rooms", &RoomList::rooms).maxLength(32).stream();
    types.add<EnemyKind>();
}
```

### `include/engine/reflection/TypeRegistry.hpp`

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

## Depends on

- [ENG-17](ENG-17.md) — IStorage and paged SparseSet<T>
- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError

## Unlocks

- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy
- [ENG-23](ENG-23.md) — YAML wrappers and field codecs
- [ENG-25](ENG-25.md) — Engine components and their registration
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events
- [ENG-39](ENG-39.md) — Replication: NetworkIdMap, net components, snapshot and input systems
- [ENG-54](ENG-54.md) — Unit tests: ECS, reflection, commands, hooks
- [SRV-02](SRV-02.md) — Handshake: player name, format fingerprint, ServerFull / FormatMismatch, TCP ↔ UDP link
- [SRV-04](SRV-04.md) — Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits

## Read first

- [ADR 0027](../adr/0027-reflection-by-name.md)
- [ADR 0036](../adr/0036-one-add-and-type-categories.md)
- [reference 04](../engine/reference/04-reflection.md)

## To implement

- [ ] `TypeCategory`, `TypeName<T>`, `HasTypeName`, `typeKey<T>`
- [ ] Macros `ENGINE_COMPONENT`, `ENGINE_EVENT`, `ENGINE_MESSAGE`, `ENGINE_STRUCT`, `ENGINE_ENUM` (qualified name → last part)
- [ ] `FieldKind` (with List, Struct), `FieldInfo` (maxLength, element kind), `TypeFlags`, `Transport`, `TypeInfo`
- [ ] `FieldsBuilder<T>` (`field` with deducing this, `maxLength`) and the five builders with their options
- [ ] `TypeRegistry::add<T>()` choosing the builder with `if constexpr`; `id`, `find`, `findByKey`, `info`, `all`, `messages`, `formatFingerprint`, `closestName`, `clearGameTypes`

## Done when

- Tests: each category registers; wrong option fails to compile (compile-fail test); DuplicateName; fingerprint changes when a field changes

## Watch out

- `static_assert` message must name the five macros
- Engine types register before the game's

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
