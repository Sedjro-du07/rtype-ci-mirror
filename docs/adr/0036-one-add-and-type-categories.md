# ADR 0036: One `add<T>()`, the type category comes from the macro

- Status: Accepted
- Date: 2026-10-14
- Deciders: the R-Type team
- Discussion id: SRV-A (Server part, round 1)
- Amends: [ADR 0027](0027-reflection-by-name.md)

## Context

ADR 0027 had three registration methods (`add`, `addEvent`, `addEnum`) and one `TypeBuilder` for
everything. Network messages (ADR 0037) add a fourth kind of type, and messages that carry lists of
small records (the lobby's room list) need a fifth: a plain struct used inside other types. With one
method per kind, the game said the kind twice (macro and method) and could contradict itself.

## Decision

- The macro placed after a type gives its **category**: `ENGINE_COMPONENT`, `ENGINE_EVENT`,
  `ENGINE_MESSAGE`, `ENGINE_STRUCT` or `ENGINE_ENUM`. `TypeName<T>` carries the name, the key
  (`fnv1a64(name)`) and the `TypeCategory`.
- `TypeRegistry::add<T>()` is the **only** registration method. It reads the category at compile
  time and returns the matching builder: `ComponentBuilder`, `EventBuilder`, `MessageBuilder`,
  `StructBuilder` or `EnumBuilder`. A type without a macro fails with a `static_assert`.
- The builders that describe fields inherit `field()` from `FieldsBuilder<T>` (written once, with
  C++23 *deducing this* so chaining keeps the concrete builder). Options exist only where they make
  sense: `flags`, `yaml` on components; `maxLength`, `unreliable`, `reliable`, `stream` on messages;
  `binary` on components and messages. A wrong option is a compile error.
- Type macros are used outside any namespace and accept a qualified name; the registered name is the
  last part (`rtype::Health` → `Health`).
- `FieldKind` gains `List` (`std::vector<T>`) and `Struct` (a type registered with `ENGINE_STRUCT`).
  A list is encoded as its count followed by its elements; the count is checked against the limit
  before any allocation.

## Alternatives considered

- **Keep one method per kind:** the kind is stated twice and the two can disagree.
- **Pass the kind as a runtime argument (`add<T>(Kind::Message)`):** options could no longer be
  checked at compile time.
- **Hand-written codecs for nested structs:** every lobby or weapon description would need its own
  `binary()` and `yaml()`.

## Consequences

- Game registration code is one method: `types.add<T>()`, then the options of its kind.
- `TypeBuilder<T>` is replaced by the five builders, in the same header (`reflection/TypeBuilder.hpp`).
