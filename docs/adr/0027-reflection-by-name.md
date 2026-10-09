# ADR 0027: Types are identified by name hash and described by registration

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 4

## Context

Cross-module identity cannot use `typeid` or static counters (ADR 0005). The loader, editor, network and scripts all need field-level descriptions.

## Decision

`ENGINE_COMPONENT`, `ENGINE_EVENT` and `ENGINE_ENUM` give each type a name; its key is the compile-time FNV-1a hash of that name. `TypeRegistry::add<T>()` returns a `TypeBuilder` to declare fields and flags (`Replicated`, `Transient`, `EditorHidden`, `Tag`), with custom YAML or binary codecs when needed. `TypeInfo` carries construct, destroy, copy, storage factory, YAML and binary codecs. `ByteWriter` and `ByteReader` are bounds-checked; byte order is fixed by the protocol RFC.

## Alternatives considered

- **Code generation (Unreal UHT style):** a build tool to write and maintain for six weeks.
- **Third-party reflection (Boost.PFR, RTTR):** no field names or flags, or a heavy dependency.

## Consequences

- One registration line per field feeds files, editor, network and scripts.
- Name or hash collisions are boot errors.
