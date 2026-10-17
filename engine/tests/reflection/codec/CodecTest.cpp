/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** CodecTest
*/
// SRV-04 — Tests for the generic binary Codec
// Done-when: round-trip for every FieldKind; fuzz: random bytes never crash.

#include "reflection/Codec.hpp"

#include "engine/core/math/Color.hpp"
#include "engine/core/math/Rect.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/reflection/ByteStream.hpp"
#include "engine/reflection/TypeInfo.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

using namespace engine;
using namespace engine::codec;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static WireContext nullWire() {
    WireContext w;
    w.networkIdOf = [](Entity) -> uint32_t { return 0u; };
    w.entityOf = [](uint32_t) -> Entity { return Entity::null(); };
    return w;
}

static FieldInfo makeField(FieldKind kind, size_t offset = 0, const EnumInfo* enumInfo = nullptr) {
    FieldInfo f;
    f.kind = kind;
    f.offset = offset;
    f.maxLength = 0; // use defaults (256 / 64)
    f.enumInfo = enumInfo;
    return f;
}

/// Round-trip a single field: encode it then decode it, comparing the results.
template <class T>
static T roundTripField(FieldKind, const T& value, const FieldInfo& field, const WireContext& wire) {
    std::vector<std::byte> buf;
    ByteWriter w(buf, 4096);

    const std::byte* src = reinterpret_cast<const std::byte*>(&value);
    encodeField(field, src, w, wire);
    REQUIRE_FALSE(w.overflowed());

    T result{};
    std::byte* dst = reinterpret_cast<std::byte*>(&result);
    ByteReader r({buf.data(), buf.size()});
    auto res = decodeField(field, dst, r, wire);
    REQUIRE(res);
    return result;
}

// ---------------------------------------------------------------------------
// Primitive FieldKind round-trips
// ---------------------------------------------------------------------------

TEST_CASE("Codec: Bool round-trip", "[codec][reflection]") {
    auto f = makeField(FieldKind::Bool);
    CHECK(roundTripField(FieldKind::Bool, true, f, nullWire()) == true);
    CHECK(roundTripField(FieldKind::Bool, false, f, nullWire()) == false);
}

TEST_CASE("Codec: Int32 round-trip", "[codec][reflection]") {
    auto f = makeField(FieldKind::Int32);
    CHECK(roundTripField(FieldKind::Int32, int32_t{-42}, f, nullWire()) == -42);
    CHECK(roundTripField(FieldKind::Int32, int32_t{0}, f, nullWire()) == 0);
    CHECK(roundTripField(FieldKind::Int32, int32_t{2147483647}, f, nullWire()) == 2147483647);
}

TEST_CASE("Codec: UInt32 round-trip", "[codec][reflection]") {
    auto f = makeField(FieldKind::UInt32);
    CHECK(roundTripField(FieldKind::UInt32, uint32_t{0xDEADBEEF}, f, nullWire()) == 0xDEADBEEFu);
}

TEST_CASE("Codec: Float round-trip", "[codec][reflection]") {
    auto f = makeField(FieldKind::Float);
    float result = roundTripField(FieldKind::Float, 3.14f, f, nullWire());
    CHECK(result == Catch::Approx(3.14f));
}

TEST_CASE("Codec: Float quantized round-trip", "[codec][reflection]") {
    FieldInfo f = makeField(FieldKind::Float);
    f.quantMin = 0.f;
    f.quantMax = 1000.f;
    // 360.0 in [0, 1000] → u16 → back
    float result = roundTripField(FieldKind::Float, 360.f, f, nullWire());
    // quantization error < 1000/65535 ≈ 0.015
    CHECK(result == Catch::Approx(360.f).margin(0.02f));
}

TEST_CASE("Codec: Double round-trip", "[codec][reflection]") {
    auto f = makeField(FieldKind::Double);
    double result = roundTripField(FieldKind::Double, 2.718281828, f, nullWire());
    CHECK(result == Catch::Approx(2.718281828));
}

TEST_CASE("Codec: String round-trip", "[codec][reflection]") {
    auto f = makeField(FieldKind::String);
    std::string result = roundTripField(FieldKind::String, std::string{"Hello R-Type"}, f, nullWire());
    CHECK(result == "Hello R-Type");
}

TEST_CASE("Codec: Vec2 round-trip", "[codec][reflection]") {
    auto f = makeField(FieldKind::Vec2);
    Vec2 result = roundTripField(FieldKind::Vec2, Vec2{40.f, 90.f}, f, nullWire());
    CHECK(result.x == Catch::Approx(40.f));
    CHECK(result.y == Catch::Approx(90.f));
}

TEST_CASE("Codec: Vec2 quantized round-trip", "[codec][reflection]") {
    FieldInfo f = makeField(FieldKind::Vec2);
    f.quantMin = -1000.f;
    f.quantMax = 1000.f;
    Vec2 result = roundTripField(FieldKind::Vec2, Vec2{-123.4f, 567.8f}, f, nullWire());
    CHECK(result.x == Catch::Approx(-123.4f).margin(0.04f));
    CHECK(result.y == Catch::Approx(567.8f).margin(0.04f));
}

TEST_CASE("Codec: Rect round-trip", "[codec][reflection]") {
    auto f = makeField(FieldKind::Rect);
    Rect result = roundTripField(FieldKind::Rect, Rect{1.f, 2.f, 3.f, 4.f}, f, nullWire());
    CHECK(result.x == Catch::Approx(1.f));
    CHECK(result.y == Catch::Approx(2.f));
    CHECK(result.w == Catch::Approx(3.f));
    CHECK(result.h == Catch::Approx(4.f));
}

TEST_CASE("Codec: Color round-trip", "[codec][reflection]") {
    auto f = makeField(FieldKind::Color);
    Color result = roundTripField(FieldKind::Color, Color{255, 128, 64, 32}, f, nullWire());
    CHECK(result.r == 255);
    CHECK(result.g == 128);
    CHECK(result.b == 64);
    CHECK(result.a == 32);
}

TEST_CASE("Codec: Entity encodes as NetworkId 0 when not replicated", "[codec][reflection]") {
    auto f = makeField(FieldKind::Entity);
    auto wire = nullWire();
    Entity e{1, 0};
    Entity result = roundTripField(FieldKind::Entity, e, f, wire);
    // networkIdOf returns 0 → entityOf(0) not called → Entity::null()
    CHECK(result.isNull());
}

TEST_CASE("Codec: Entity NetworkId round-trip via WireContext", "[codec][reflection]") {
    auto f = makeField(FieldKind::Entity);
    WireContext wire;
    wire.networkIdOf = [](Entity e) -> uint32_t { return e.index + 1; };
    wire.entityOf = [](uint32_t id) -> Entity { return Entity{id - 1, 0}; };

    Entity e{42, 0};
    Entity result = roundTripField(FieldKind::Entity, e, f, wire);
    CHECK(result.index == 42u);
}

TEST_CASE("Codec: Enum round-trip with known value", "[codec][reflection]") {
    EnumInfo info;
    info.values = {{"A", 0}, {"B", 1}, {"C", 2}};
    auto f = makeField(FieldKind::Enum, 0, &info);

    int32_t val = 1;
    int32_t result = roundTripField(FieldKind::Enum, val, f, nullWire());
    CHECK(result == 1);
}

TEST_CASE("Codec: Enum decode fails for unknown value", "[codec][reflection]") {
    EnumInfo info;
    info.values = {{"A", 0}, {"B", 1}};
    auto f = makeField(FieldKind::Enum, 0, &info);

    // Encode value 99 (not in enum)
    std::vector<std::byte> buf;
    ByteWriter w(buf, 16);
    int32_t bad = 99;
    encodeField(f, reinterpret_cast<const std::byte*>(&bad), w, nullWire());

    int32_t out = 0;
    ByteReader r({buf.data(), buf.size()});
    auto res = decodeField(f, reinterpret_cast<std::byte*>(&out), r, nullWire());
    CHECK_FALSE(res);
}

TEST_CASE("Codec: Bool decode fails for value != 0 or 1", "[codec][reflection]") {
    // Craft a raw byte of 2
    std::vector<std::byte> buf = {std::byte{0x02}};
    bool out = false;
    auto f = makeField(FieldKind::Bool);
    ByteReader r({buf.data(), buf.size()});
    auto res = decodeField(f, reinterpret_cast<std::byte*>(&out), r, nullWire());
    CHECK_FALSE(res);
}

// ---------------------------------------------------------------------------
// Fuzz: random bytes must not crash or over-allocate
// ---------------------------------------------------------------------------

TEST_CASE("Codec: random bytes never crash on decode (fuzz light)", "[codec][reflection][fuzz]") {
    // Feed 256 different single-byte patterns through each FieldKind decoder.
    std::vector<FieldKind> kinds = {
        FieldKind::Bool, FieldKind::Int32, FieldKind::UInt32, FieldKind::Float,  FieldKind::Double, FieldKind::String,
        FieldKind::Vec2, FieldKind::Rect,  FieldKind::Color,  FieldKind::Entity, FieldKind::Enum,
    };

    EnumInfo info;
    info.values = {{"X", 0}};

    for (auto kind : kinds) {
        FieldInfo f = makeField(kind, 0, kind == FieldKind::Enum ? &info : nullptr);
        // Very short buffer — most reads will underrun; must not crash
        for (int b = 0; b < 256; ++b) {
            std::vector<std::byte> buf(8, static_cast<std::byte>(b));
            ByteReader r({buf.data(), buf.size()});
            std::vector<std::byte> obj(64, std::byte{0});
            (void)decodeField(f, obj.data(), r, nullWire());
            // No assertion: just must not crash or throw
        }
    }
}

// ---------------------------------------------------------------------------
// fingerprint
// ---------------------------------------------------------------------------

TEST_CASE("Codec: fingerprint is deterministic and order-sensitive", "[codec][reflection]") {
    TypeInfo t1;
    t1.name = "Health";
    FieldInfo f1;
    f1.name = "hp";
    f1.kind = FieldKind::Int32;
    t1.fields = {f1};

    TypeInfo t2;
    t2.name = "Position";
    FieldInfo f2;
    f2.name = "pos";
    f2.kind = FieldKind::Vec2;
    t2.fields = {f2};

    const TypeInfo* arr1[] = {&t1, &t2};
    const TypeInfo* arr2[] = {&t2, &t1};

    uint64_t fp1a = fingerprint(arr1);
    uint64_t fp1b = fingerprint(arr1);
    uint64_t fp2 = fingerprint(arr2);

    CHECK(fp1a == fp1b); // deterministic
    CHECK(fp1a != fp2);  // order-sensitive
    CHECK(fp1a != 0);
}
