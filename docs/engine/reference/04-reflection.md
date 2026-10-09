# 4 · Reflection and serialization

Status: **Locked**. ADRs 0005, 0014, 0018, 0027.
Folder: `engine/include/engine/reflection/`, `engine/src/reflection/`.

## Role

Teach the engine the types a game invents, so it can store them (ECS), read and write them (YAML),
show them (editor, console), send them (network) and expose them (scripts) without being compiled
against them. Reflection by explicit registration, no code generation.

---

## Type keys — `reflection/TypeKey.hpp`

Cross-module identity never uses `typeid` or static counters (ADR 0005). Each component, event or
enum declares its name once; the key is the compile-time hash of that name.

```cpp
template <class T> struct TypeName;               // specialised by the macros below

#define ENGINE_COMPONENT(Type)  /* TypeName<Type>::value = "Type", TypeName<Type>::key = fnv1a64("Type") */
#define ENGINE_EVENT(Type)      /* same, for event structs (block 6) */
#define ENGINE_ENUM(Type, ...)  /* same, plus the list of enumerator names for YAML and the inspector */

template <class T> constexpr uint64_t typeKey() noexcept;   // TypeName<T>::key
using ComponentId = uint32_t;                                // dense index assigned at registration
```

A second registration with the same name, or two names with the same hash, is a boot error
(`DuplicateName`).

## `enum class FieldKind` and `struct FieldInfo` — `reflection/TypeInfo.hpp`

```cpp
enum class FieldKind : uint8_t {
    Bool, Int32, UInt32, Int64, Float, Double, String,
    Vec2, Rect, Color, Entity,                          // Entity: written as the target's Uuid in files
    Texture, Sound, Music, Font, Script,                // asset handles: written as paths in files
    Enum, Custom
};

struct FieldInfo {
    std::string         name;          // YAML key and inspector label
    FieldKind           kind;
    size_t              offset;        // offsetof within the component
    const EnumInfo*     enumInfo = nullptr;   // when kind == Enum
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
};

struct TypeInfo {
    ComponentId               id;
    uint64_t                  key;          // fnv1a64(name)
    std::string               name;
    size_t                    size, align;
    uint32_t                  flags;
    std::vector<FieldInfo>    fields;
    void (*construct)(void* dst);                         // default-construct
    void (*destroy)(void* obj) noexcept;
    void (*copy)(void* dst, const void* src);             // copy-construct
    std::unique_ptr<IStorage> (*makeStorage)(ComponentId);// creates SparseSet<T>
    Result<void> (*fromYaml)(void* obj, const YamlNode&, LoadContext&); // generated from fields, or custom
    void         (*toYaml)(const void* obj, YamlWriter&, SaveContext&);
    void         (*encode)(const void* obj, ByteWriter&); // generated from fields, or custom
    Result<void> (*decode)(void* obj, ByteReader&);
};
```

`YamlNode` / `YamlWriter` are thin engine wrappers so that yaml-cpp stays out of game headers.

## `class TypeBuilder<T>` — `reflection/TypeBuilder.hpp`

```cpp
template <class T>
class TypeBuilder {
public:
    template <class M> TypeBuilder& field(std::string_view name, M T::*member, std::string_view tooltip = {});
    TypeBuilder& flags(uint32_t f);                                   // Replicated | Transient …
    TypeBuilder& yaml(FromYamlFn read, ToYamlFn write);               // custom YAML instead of field-by-field
    TypeBuilder& binary(EncodeFn encode, DecodeFn decode);            // custom wire format (e.g. quantised)
};
```

`field()` deduces `FieldKind` from the member type; an unsupported member type is a compile error
telling the author to use `yaml()` / `binary()`.

## `class TypeRegistry` — `reflection/TypeRegistry.hpp`

```cpp
class TypeRegistry {
public:
    template <class T> TypeBuilder<T> add();                // name from ENGINE_COMPONENT(T)
    template <class E> void addEvent();                     // name from ENGINE_EVENT(E)
    template <class En> void addEnum();                     // name and values from ENGINE_ENUM

    template <class T> ComponentId id() const;              // hash lookup by typeKey<T>(), O(1)
    const TypeInfo* find(std::string_view name) const;      // nullptr if unknown (loader suggests a close name)
    const TypeInfo& info(ComponentId id) const;
    std::span<const TypeInfo> all() const;                  // inspector, console "types"
    std::string closestName(std::string_view unknown) const;// "Helth" → "Health"
    void clearGameTypes();                                  // before unloading a game module (ADR 0014)
};
```

Engine components are registered first (block 13), then `IGameModule::registerTypes`.

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

Byte order and the encoding of each wire field are fixed by the protocol RFC (Network part).
These classes guarantee bounds and limits so that a malformed packet can never read or allocate
out of range.
