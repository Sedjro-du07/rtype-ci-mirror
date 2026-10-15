# ENG-24 · ByteWriter / ByteReader: bounds-checked, bit packing

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **04 Reflection** · **Wave 2** · status and owner live on the board.

## Goal

Bounds-checked byte writing and reading: a malformed packet can never read or allocate out of range.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/reflection/ByteStream.hpp`
- `src/reflection/ByteStream.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/reflection/ByteStream.hpp`

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

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError

## Unlocks

- [ENG-38](ENG-38.md) — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts
- [SRV-04](SRV-04.md) — Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits

## Read first

- [ADR 0027](../adr/0027-reflection-by-name.md)
- [RFC §5](../network/rfc.md) (MSEP v1.0)

## To implement

- [ ] `ByteWriter` with a max size and `overflowed`
- [ ] `ByteReader` returning `std::optional`, `failed`
- [ ] Bit packing (`writeBits`, `bits`)

## Done when

- Tests: every read past the end fails cleanly; string longer than the limit fails

## Watch out

- Big-endian on the wire, floats via `std::bit_cast<uint32_t>` (network D1): keep it in one place

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
