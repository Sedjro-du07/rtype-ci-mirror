# `engine/src/reflection/`

Headers in this folder are **private** to the engine: they are not under `include/`, so games cannot include them.

Placeholder created from the architecture. Replace this file's list as files are written.

| File to create | Task | Details |
|---|---|---|
| `ByteStream.cpp` | ENG-24 — ByteWriter / ByteReader: bounds-checked, bit packing | `docs/tasks/ENG-24.md` |
| `Codec.cpp` | SRV-04 — Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits | `docs/tasks/SRV-04.md` |
| `Codec.hpp` | SRV-04 — Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits | `docs/tasks/SRV-04.md` |
| `TypeRegistry.cpp` | ENG-22 — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum) | `docs/tasks/ENG-22.md` |
| `Yaml.cpp` | ENG-23 — YAML wrappers and field codecs | `docs/tasks/ENG-23.md` |

Design: `docs/engine/reference/`. Rules: `docs/engine/folder-tree.md`.
