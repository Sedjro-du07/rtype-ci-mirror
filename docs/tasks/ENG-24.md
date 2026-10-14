# ENG-24 · ByteWriter / ByteReader: bounds-checked, bit packing

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **04 Reflection** · status and owner live on the board.

## Goal

Bounds-checked byte writing and reading: a malformed packet can never read or allocate out of range.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/reflection/ByteStream.hpp`
- `src/reflection/ByteStream.cpp`

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError

## Unlocks

- [ENG-38](ENG-38.md) — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts
- [SRV-04](SRV-04.md) — Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits

## Read first

- [ADR 0027](../adr/0027-reflection-by-name.md)

## To implement

- [ ] `ByteWriter` with a max size and `overflowed`
- [ ] `ByteReader` returning `std::optional`, `failed`
- [ ] Bit packing (`writeBits`, `bits`)

## Done when

- Tests: every read past the end fails cleanly; string longer than the limit fails

## Watch out

- Byte order is fixed by the protocol RFC: keep it in one place

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
