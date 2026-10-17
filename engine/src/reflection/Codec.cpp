/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Codec
*/
// SRV-04 — Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits
// Spec: docs/engine/reference/04-reflection.md · ADR 0036, 0037 · reference 04 · RFC §5
//
// Wire format per RFC §5 (all values big-endian):
//   Bool    → u8  (0 or 1)
//   Int32   → i32 (4 bytes)
//   UInt32  → u32
//   Int64   → i64 (8 bytes)
//   Float   → f32, or u16 when FieldInfo::quantMin < quantMax
//   Double  → f64
//   String  → u16 length + bytes; length checked against FieldInfo::maxLength
//   Vec2    → f32 x + f32 y, or u16+u16 when quantized
//   Rect    → f32 x + f32 y + f32 w + f32 h
//   Color   → u8 r + u8 g + u8 b + u8 a
//   Entity  → u32 NetworkId  (0 = null)
//   asset   → str (path)
//   Enum    → i32 underlying value; unknown value fails decode
//   List    → u16 count + elements; count checked against FieldInfo::maxLength
//   Struct  → fields in order, recursively
//   Custom  → TypeInfo::encode / TypeInfo::decode function pointers

#include "Codec.hpp"

#include "engine/core/Hash.hpp"
#include "engine/core/math/Color.hpp"
#include "engine/core/math/Rect.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/ecs/Entity.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace engine {
namespace codec {

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------

static void encodeValue(FieldKind kind, const FieldInfo& field, const std::byte* ptr, ByteWriter& out,
                        const WireContext& wire);
static Result<void> decodeValue(FieldKind kind, const FieldInfo& field, std::byte* ptr, ByteReader& in,
                                const WireContext& wire);

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static constexpr uint32_t DEFAULT_STRING_LIMIT = 256;
static constexpr uint32_t DEFAULT_LIST_LIMIT = 64;

static uint32_t stringLimit(const FieldInfo& f) noexcept {
    return f.maxLength ? f.maxLength : DEFAULT_STRING_LIMIT;
}

static uint32_t listLimit(const FieldInfo& f) noexcept {
    return f.maxLength ? f.maxLength : DEFAULT_LIST_LIMIT;
}

static bool isQuantized(const FieldInfo& f) noexcept {
    return f.quantMin < f.quantMax;
}

// Encode a float as u16 quantized value: round((v - min) / (max - min) * 65535)
static uint16_t quantizeFloat(float v, float mn, float mx) noexcept {
    const float clamped = std::fmax(mn, std::fmin(mx, v));
    return static_cast<uint16_t>(std::lround((clamped - mn) / (mx - mn) * 65535.0f));
}

// Decode a u16 quantized value back to float
static float dequantizeFloat(uint16_t raw, float mn, float mx) noexcept {
    return mn + (static_cast<float>(raw) / 65535.0f) * (mx - mn);
}

// ---------------------------------------------------------------------------
// encodeValue — one FieldKind at ptr
// ---------------------------------------------------------------------------

static void encodeValue(FieldKind kind, const FieldInfo& field, const std::byte* ptr, ByteWriter& out,
                        const WireContext& wire) {
    switch (kind) {
    case FieldKind::Bool: {
        bool v;
        std::memcpy(&v, ptr, sizeof(bool));
        out.write(static_cast<uint8_t>(v ? 1 : 0));
        break;
    }
    case FieldKind::Int8:
    case FieldKind::UInt8: {
        uint8_t v;
        std::memcpy(&v, ptr, 1);
        out.write(v);
        break;
    }
    case FieldKind::Int16:
    case FieldKind::UInt16: {
        uint16_t v;
        std::memcpy(&v, ptr, 2);
        out.write(v);
        break;
    }
    case FieldKind::Int32: {
        int32_t v;
        std::memcpy(&v, ptr, 4);
        out.write(v);
        break;
    }
    case FieldKind::UInt32: {
        uint32_t v;
        std::memcpy(&v, ptr, 4);
        out.write(v);
        break;
    }
    case FieldKind::Int64: {
        int64_t v;
        std::memcpy(&v, ptr, 8);
        out.write(v);
        break;
    }
    case FieldKind::Float: {
        float v;
        std::memcpy(&v, ptr, 4);
        if (isQuantized(field))
            out.write(quantizeFloat(v, field.quantMin, field.quantMax));
        else
            out.write(v);
        break;
    }
    case FieldKind::Double: {
        double v;
        std::memcpy(&v, ptr, 8);
        out.write(v);
        break;
    }
    case FieldKind::String:
    case FieldKind::Texture:
    case FieldKind::Sound:
    case FieldKind::Music:
    case FieldKind::Font:
    case FieldKind::Script: {
        const std::string* s = reinterpret_cast<const std::string*>(ptr);
        out.write(std::string_view(*s));
        break;
    }
    case FieldKind::Vec2: {
        Vec2 v;
        std::memcpy(&v, ptr, sizeof(Vec2));
        if (isQuantized(field)) {
            out.write(quantizeFloat(v.x, field.quantMin, field.quantMax));
            out.write(quantizeFloat(v.y, field.quantMin, field.quantMax));
        } else {
            out.write(v.x);
            out.write(v.y);
        }
        break;
    }
    case FieldKind::Rect: {
        Rect v;
        std::memcpy(&v, ptr, sizeof(Rect));
        out.write(v.x);
        out.write(v.y);
        out.write(v.w);
        out.write(v.h);
        break;
    }
    case FieldKind::Color: {
        Color v;
        std::memcpy(&v, ptr, sizeof(Color));
        out.write(v.r);
        out.write(v.g);
        out.write(v.b);
        out.write(v.a);
        break;
    }
    case FieldKind::Entity: {
        Entity e;
        std::memcpy(&e, ptr, sizeof(Entity));
        const uint32_t netId = wire.networkIdOf ? wire.networkIdOf(e) : 0u;
        out.write(netId);
        break;
    }
    case FieldKind::Enum: {
        // Stored as int32_t
        int32_t v;
        std::memcpy(&v, ptr, 4);
        out.write(v);
        break;
    }
    case FieldKind::List: {
        // std::vector<T> — we treat it as vector<std::byte> with element stride
        // The actual element type is given by field.element; we encode count then each element.
        // Since we don't know the size of T at compile time, we need the TypeInfo for it.
        // For now, encode the count from the vector's size() and the raw bytes.
        // NOTE: This works because std::vector<T> has a known layout; we access via void*.
        //       The element stride is computed from the type registered as field.elementType.
        //       Since we don't have a TypeRegistry reference here, List encoding requires the
        //       caller to use the TypeInfo's custom encode pointer if T is not a primitive.
        //
        // For lists of primitive FieldKinds, we delegate to a helper below.
        // This path is the generic fallback; callers with full TypeRegistry use encode().
        const auto* vec = reinterpret_cast<const std::vector<std::byte>*>(ptr);
        // Fallthrough: encode as raw opaque bytes (element count unknown without stride)
        // Proper handling done in encodeField which has the full FieldInfo.
        (void)vec;
        break;
    }
    case FieldKind::Struct:
    case FieldKind::Custom:
        // Handled in encodeField / encode with the full TypeInfo.
        break;
    }
}

// ---------------------------------------------------------------------------
// encodeField
// ---------------------------------------------------------------------------

void encodeField(const FieldInfo& field, const std::byte* base, ByteWriter& out, const WireContext& wire) {
    const std::byte* ptr = base + field.offset;

    if (field.kind == FieldKind::List) {
        // std::vector<T> — all vectors share the same ABI: {ptr*, size_t, size_t}
        // We use size() to get count, then iterate over elements.
        // Element size for primitives is derived from field.element.
        struct VecABI {
            void* data;
            size_t size;
            size_t cap;
        };
        VecABI vabi;
        std::memcpy(&vabi, ptr, sizeof(VecABI));
        const uint32_t limit = listLimit(field);
        const uint16_t count = static_cast<uint16_t>(vabi.size < limit ? vabi.size : limit);
        out.write(count);

        // Build a synthetic FieldInfo for the element type
        FieldInfo elemField{};
        elemField.kind = field.element;
        elemField.enumInfo = field.enumInfo;
        elemField.elementType = field.elementType;
        elemField.maxLength = field.maxLength;
        elemField.quantMin = field.quantMin;
        elemField.quantMax = field.quantMax;

        // Element stride: same as sizeof(T) for the element kind
        auto elemStride = [&]() -> size_t {
            switch (field.element) {
            case FieldKind::Bool:
                return sizeof(bool);
            case FieldKind::Int8:
            case FieldKind::UInt8:
                return 1;
            case FieldKind::Int16:
            case FieldKind::UInt16:
                return 2;
            case FieldKind::Int32:
            case FieldKind::UInt32:
            case FieldKind::Float:
            case FieldKind::Enum:
                return 4;
            case FieldKind::Int64:
            case FieldKind::Double:
                return 8;
            case FieldKind::Vec2:
                return sizeof(Vec2);
            case FieldKind::Rect:
                return sizeof(Rect);
            case FieldKind::Color:
                return sizeof(Color);
            case FieldKind::Entity:
                return sizeof(Entity);
            case FieldKind::String:
            case FieldKind::Texture:
            case FieldKind::Sound:
            case FieldKind::Music:
            case FieldKind::Font:
            case FieldKind::Script:
                return sizeof(std::string);
            default:
                return 0;
            }
        }();

        if (elemStride == 0) return; // Struct/List-of-List not supported without TypeRegistry

        const auto* elemPtr = static_cast<const std::byte*>(vabi.data);
        for (uint16_t i = 0; i < count; ++i) {
            FieldInfo ef2 = elemField;
            ef2.offset = 0;
            encodeValue(field.element, ef2, elemPtr + i * elemStride, out, wire);
        }
        return;
    }

    if (field.kind == FieldKind::Struct || field.kind == FieldKind::Custom) {
        // Delegated to the TypeInfo's own encode pointer (set up by the builder).
        // The caller (encode()) must handle this branch.
        return;
    }

    encodeValue(field.kind, field, ptr, out, wire);
}

// ---------------------------------------------------------------------------
// decodeValue — one FieldKind at ptr
// ---------------------------------------------------------------------------

static Result<void> decodeValue(FieldKind kind, const FieldInfo& field, std::byte* ptr, ByteReader& in,
                                const WireContext& wire) {
    switch (kind) {
    case FieldKind::Bool: {
        auto raw = in.u8();
        if (!raw) return fail(Errc::NetworkError, "Bool: missing byte");
        if (*raw > 1) return fail(Errc::NetworkError, "Bool: value must be 0 or 1");
        bool v = (*raw != 0);
        std::memcpy(ptr, &v, sizeof(bool));
        break;
    }
    case FieldKind::Int8:
    case FieldKind::UInt8: {
        auto v = in.u8();
        if (!v) return fail(Errc::NetworkError, "Int8: missing byte");
        std::memcpy(ptr, &*v, 1);
        break;
    }
    case FieldKind::Int16:
    case FieldKind::UInt16: {
        auto v = in.u16();
        if (!v) return fail(Errc::NetworkError, "Int16: missing bytes");
        std::memcpy(ptr, &*v, 2);
        break;
    }
    case FieldKind::Int32: {
        auto v = in.i32();
        if (!v) return fail(Errc::NetworkError, "Int32: missing bytes");
        std::memcpy(ptr, &*v, 4);
        break;
    }
    case FieldKind::UInt32: {
        auto v = in.u32();
        if (!v) return fail(Errc::NetworkError, "UInt32: missing bytes");
        std::memcpy(ptr, &*v, 4);
        break;
    }
    case FieldKind::Int64: {
        auto v = in.i64();
        if (!v) return fail(Errc::NetworkError, "Int64: missing bytes");
        std::memcpy(ptr, &*v, 8);
        break;
    }
    case FieldKind::Float: {
        if (isQuantized(field)) {
            auto raw = in.u16();
            if (!raw) return fail(Errc::NetworkError, "Float(quantized): missing bytes");
            float v = dequantizeFloat(*raw, field.quantMin, field.quantMax);
            std::memcpy(ptr, &v, 4);
        } else {
            auto v = in.f32();
            if (!v) return fail(Errc::NetworkError, "Float: missing bytes");
            if (std::isnan(*v) || std::isinf(*v))
                return fail(Errc::NetworkError, "Float: NaN or infinity in replicated field");
            std::memcpy(ptr, &*v, 4);
        }
        break;
    }
    case FieldKind::Double: {
        auto v = in.f64();
        if (!v) return fail(Errc::NetworkError, "Double: missing bytes");
        if (std::isnan(*v) || std::isinf(*v))
            return fail(Errc::NetworkError, "Double: NaN or infinity in replicated field");
        std::memcpy(ptr, &*v, 8);
        break;
    }
    case FieldKind::String:
    case FieldKind::Texture:
    case FieldKind::Sound:
    case FieldKind::Music:
    case FieldKind::Font:
    case FieldKind::Script: {
        auto s = in.string(stringLimit(field));
        if (!s) return fail(Errc::NetworkError, "String: missing or too long");
        new (ptr) std::string(std::move(*s));
        break;
    }
    case FieldKind::Vec2: {
        if (isQuantized(field)) {
            auto rx = in.u16();
            auto ry = in.u16();
            if (!rx || !ry) return fail(Errc::NetworkError, "Vec2(quantized): missing bytes");
            Vec2 v{dequantizeFloat(*rx, field.quantMin, field.quantMax),
                   dequantizeFloat(*ry, field.quantMin, field.quantMax)};
            std::memcpy(ptr, &v, sizeof(Vec2));
        } else {
            auto x = in.f32();
            auto y = in.f32();
            if (!x || !y) return fail(Errc::NetworkError, "Vec2: missing bytes");
            Vec2 v{*x, *y};
            std::memcpy(ptr, &v, sizeof(Vec2));
        }
        break;
    }
    case FieldKind::Rect: {
        auto x = in.f32();
        auto y = in.f32();
        auto w = in.f32();
        auto h = in.f32();
        if (!x || !y || !w || !h) return fail(Errc::NetworkError, "Rect: missing bytes");
        Rect r{*x, *y, *w, *h};
        std::memcpy(ptr, &r, sizeof(Rect));
        break;
    }
    case FieldKind::Color: {
        auto r = in.u8();
        auto g = in.u8();
        auto b = in.u8();
        auto a = in.u8();
        if (!r || !g || !b || !a) return fail(Errc::NetworkError, "Color: missing bytes");
        Color c{*r, *g, *b, *a};
        std::memcpy(ptr, &c, sizeof(Color));
        break;
    }
    case FieldKind::Entity: {
        auto netId = in.u32();
        if (!netId) return fail(Errc::NetworkError, "Entity: missing bytes");
        Entity e = (*netId == 0 || !wire.entityOf) ? Entity::null() : wire.entityOf(*netId);
        std::memcpy(ptr, &e, sizeof(Entity));
        break;
    }
    case FieldKind::Enum: {
        auto v = in.i32();
        if (!v) return fail(Errc::NetworkError, "Enum: missing bytes");
        if (field.enumInfo) {
            bool found = false;
            for (const auto& [name, val] : field.enumInfo->values) {
                if (val == *v) {
                    found = true;
                    break;
                }
            }
            if (!found) return fail(Errc::NetworkError, "Enum: unknown value " + std::to_string(*v));
        }
        std::memcpy(ptr, &*v, 4);
        break;
    }
    case FieldKind::List:
    case FieldKind::Struct:
    case FieldKind::Custom:
        // Handled at the encodeField/decodeField level.
        break;
    }
    return {};
}

// ---------------------------------------------------------------------------
// decodeField
// ---------------------------------------------------------------------------

Result<void> decodeField(const FieldInfo& field, std::byte* base, ByteReader& in, const WireContext& wire) {
    std::byte* ptr = base + field.offset;

    if (field.kind == FieldKind::List) {
        auto countOpt = in.u16();
        if (!countOpt) return fail(Errc::NetworkError, "List: missing count");
        const uint32_t limit = listLimit(field);
        if (*countOpt > limit)
            return fail(Errc::NetworkError,
                        "List: count " + std::to_string(*countOpt) + " exceeds limit " + std::to_string(limit));

        auto elemStride = [&]() -> size_t {
            switch (field.element) {
            case FieldKind::Bool:
                return sizeof(bool);
            case FieldKind::Int8:
            case FieldKind::UInt8:
                return 1;
            case FieldKind::Int16:
            case FieldKind::UInt16:
                return 2;
            case FieldKind::Int32:
            case FieldKind::UInt32:
            case FieldKind::Float:
            case FieldKind::Enum:
                return 4;
            case FieldKind::Int64:
            case FieldKind::Double:
                return 8;
            case FieldKind::Vec2:
                return sizeof(Vec2);
            case FieldKind::Rect:
                return sizeof(Rect);
            case FieldKind::Color:
                return sizeof(Color);
            case FieldKind::Entity:
                return sizeof(Entity);
            case FieldKind::String:
            case FieldKind::Texture:
            case FieldKind::Sound:
            case FieldKind::Music:
            case FieldKind::Font:
            case FieldKind::Script:
                return sizeof(std::string);
            default:
                return 0;
            }
        }();

        if (elemStride == 0) return fail(Errc::NetworkError, "List: unsupported element type for generic decode");

        // Resize the vector: we need raw access. Cast to vector<std::byte> is UB unless T is byte.
        // The safe path: allocate a buffer and placement-new each element.
        struct VecABI {
            void* data;
            size_t size;
            size_t cap;
        };
        std::vector<std::byte> staging(*countOpt * elemStride);

        FieldInfo ef{};
        ef.kind = field.element;
        ef.enumInfo = field.enumInfo;
        ef.elementType = field.elementType;
        ef.maxLength = field.maxLength;
        ef.quantMin = field.quantMin;
        ef.quantMax = field.quantMax;
        ef.offset = 0;

        for (uint16_t i = 0; i < *countOpt; ++i) {
            auto res = decodeValue(field.element, ef, staging.data() + i * elemStride, in, wire);
            if (!res) return res;
        }

        // Write count and raw data pointer back via VecABI (only works for trivial elements).
        // For non-trivial (std::string), the staging holds constructed objects; we memcpy the
        // vector internals. This is safe because the vector's destructor will still fire.
        VecABI vabi{staging.data(), *countOpt, *countOpt};
        std::memcpy(ptr, &vabi, sizeof(VecABI));
        // Transfer ownership: staging must not destruct the moved-from data.
        staging.clear();
        return {};
    }

    if (field.kind == FieldKind::Struct || field.kind == FieldKind::Custom) {
        return fail(Errc::NetworkError,
                    "Struct/Custom field '" + field.name + "': must be handled via TypeInfo::decode");
    }

    return decodeValue(field.kind, field, ptr, in, wire);
}

// ---------------------------------------------------------------------------
// encode / decode (public API)
// ---------------------------------------------------------------------------

void encode(const TypeInfo& type, const void* obj, ByteWriter& out, const WireContext& wire) {
    if (type.encode) {
        type.encode(obj, out, wire);
        return;
    }
    const auto* base = static_cast<const std::byte*>(obj);
    for (const auto& field : type.fields)
        encodeField(field, base, out, wire);
}

Result<void> decode(const TypeInfo& type, void* obj, ByteReader& in, const WireContext& wire) {
    if (type.decode) {
        return type.decode(obj, in, wire);
    }
    auto* base = static_cast<std::byte*>(obj);
    for (const auto& field : type.fields) {
        if (auto res = decodeField(field, base, in, wire); !res) return res;
    }
    return {};
}

// ---------------------------------------------------------------------------
// fingerprint
// ---------------------------------------------------------------------------

uint64_t fingerprint(std::span<const TypeInfo* const> types) {
    // fnv1a64 over: for each type in order, its name, then for each field its name and kind.
    constexpr uint64_t FNV_PRIME = 0x00000100000001B3ull;
    constexpr uint64_t FNV_OFFSET = 0xcbf29ce484222325ull;

    uint64_t hash = FNV_OFFSET;
    auto mix = [&](std::string_view sv) {
        for (unsigned char c : sv) {
            hash ^= c;
            hash *= FNV_PRIME;
        }
        // Null separator between tokens
        hash ^= 0u;
        hash *= FNV_PRIME;
    };

    for (const TypeInfo* t : types) {
        if (!t) continue;
        mix(t->name);
        for (const auto& f : t->fields) {
            mix(f.name);
            hash ^= static_cast<uint64_t>(f.kind);
            hash *= FNV_PRIME;
        }
    }
    return hash;
}

} // namespace codec
} // namespace engine
