# SRV-04 · Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **02 Messages** · status and owner live on the board.

## Goal

One binary codec for any registered type, used by messages and replication.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/reflection/Codec.hpp`
- `src/reflection/Codec.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/reflection/Codec.hpp`

```cpp
struct WireContext {                                     // declared in reflection/TypeInfo.hpp
    std::function<uint32_t(Entity)> networkIdOf;         // 0 if not replicated (warning)
    std::function<Entity(uint32_t)> entityOf;            // Entity::null() if unknown
};

namespace codec {
void         encode(const TypeInfo& type, const void* obj, ByteWriter& out, const WireContext& wire);
Result<void> decode(const TypeInfo& type, void* obj, ByteReader& in, const WireContext& wire);   // obj default-constructed
void         encodeField(const FieldInfo& f, const std::byte* field, ByteWriter& out, const WireContext& wire);
Result<void> decodeField(const FieldInfo& f, std::byte* field, ByteReader& in, const WireContext& wire);
uint64_t     fingerprint(std::span<const TypeInfo* const> types);    // used by TypeRegistry::formatFingerprint
}
```

## Depends on

- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)
- [ENG-24](ENG-24.md) — ByteWriter / ByteReader: bounds-checked, bit packing

## Unlocks

- [ENG-39](ENG-39.md) — Replication: NetworkIdMap, net components, snapshot and input systems
- [SRV-03](SRV-03.md) — Typed messages: MessageDispatcher, send / broadcast / on<T>
- [SRV-07](SRV-07.md) — Tests: players, handshake, messages, codec, malformed input

## Read first

- [ADR 0036](../adr/0036-one-add-and-type-categories.md)
- [ADR 0037](../adr/0037-game-facing-networking.md)
- [reference 04](../engine/reference/04-reflection.md)

## To implement

- [ ] `encode` / `decode` walking `TypeInfo::fields` (private `Codec`)
- [ ] Lists: count then elements, count checked against `maxLength` (default 64) before allocating
- [ ] Text limit (default 256), structs, enums as integers
- [ ] `Entity` through `WireContext` (NetworkId, null if unknown)

## Done when

- Round-trip tests for every FieldKind; fuzz test: random bytes never crash or over-allocate

## Watch out

- A decode failure rejects the whole value
- Byte order and integer widths from the protocol RFC

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
