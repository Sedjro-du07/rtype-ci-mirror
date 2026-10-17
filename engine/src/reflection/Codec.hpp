/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Codec
*/
// SRV-04 — Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits
// Spec: docs/engine/reference/04-reflection.md · ADR 0036, 0037 · reference 04 · RFC §5

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Error.hpp"
#include "engine/reflection/ByteStream.hpp"
#include "engine/reflection/TypeInfo.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace engine {
namespace codec {

/// Encode an object of `type` into `out` using the WireContext for Entity ↔ NetworkId.
/// If TypeInfo::encode is set (custom codec), it is called instead of field-by-field.
ENGINE_API void encode(const TypeInfo& type, const void* obj, ByteWriter& out, const WireContext& wire);

/// Decode an object of `type` from `in` into `obj` (must be default-constructed by the caller).
/// Returns an error on any malformed input; `obj` may be partially written on failure.
ENGINE_API Result<void> decode(const TypeInfo& type, void* obj, ByteReader& in, const WireContext& wire);

/// Encode one field value (at field.offset inside the object) into `out`.
ENGINE_API void encodeField(const FieldInfo& field, const std::byte* base, ByteWriter& out, const WireContext& wire);

/// Decode one field value from `in` into the field (at field.offset inside the object).
ENGINE_API Result<void> decodeField(const FieldInfo& field, std::byte* base, ByteReader& in, const WireContext& wire);

/// Compute a format fingerprint: fnv1a64 over the names, field kinds and field names of all
/// types in the span, in order.  Used by TypeRegistry::formatFingerprint() (RFC §6.3.1).
ENGINE_API uint64_t fingerprint(std::span<const TypeInfo* const> types);

} // namespace codec
} // namespace engine
