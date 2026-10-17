/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ByteStreamTest
*/
// ENG-24 — Tests for ByteWriter / ByteReader

#include "engine/reflection/ByteStream.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <vector>

using namespace engine;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static ByteReader makeReader(const std::vector<std::byte>& buf)
{
    return ByteReader({buf.data(), buf.size()});
}

// ---------------------------------------------------------------------------
// ByteWriter — primitive round-trips
// ---------------------------------------------------------------------------

TEST_CASE("ByteWriter / ByteReader: u8 round-trip", "[reflection][bytestream]")
{
    std::vector<std::byte> buf;
    ByteWriter w(buf, 64);
    REQUIRE(w.write(uint8_t{0xAB}));

    auto r = makeReader(buf);
    auto v = r.u8();
    REQUIRE(v.has_value());
    CHECK(*v == 0xAB);
    CHECK(r.remaining() == 0);
}

TEST_CASE("ByteWriter / ByteReader: u16 big-endian", "[reflection][bytestream]")
{
    std::vector<std::byte> buf;
    ByteWriter w(buf, 64);
    REQUIRE(w.write(uint16_t{0x1234}));

    REQUIRE(buf.size() == 2);
    CHECK(static_cast<uint8_t>(buf[0]) == 0x12);
    CHECK(static_cast<uint8_t>(buf[1]) == 0x34);

    auto r = makeReader(buf);
    CHECK(r.u16() == uint16_t{0x1234});
}

TEST_CASE("ByteWriter / ByteReader: u32 big-endian", "[reflection][bytestream]")
{
    std::vector<std::byte> buf;
    ByteWriter w(buf, 64);
    REQUIRE(w.write(uint32_t{0xDEADBEEF}));

    REQUIRE(buf.size() == 4);
    CHECK(static_cast<uint8_t>(buf[0]) == 0xDE);
    CHECK(static_cast<uint8_t>(buf[3]) == 0xEF);

    auto r = makeReader(buf);
    CHECK(r.u32() == 0xDEADBEEFu);
}

TEST_CASE("ByteWriter / ByteReader: u64 round-trip", "[reflection][bytestream]")
{
    std::vector<std::byte> buf;
    ByteWriter w(buf, 16);
    REQUIRE(w.write(uint64_t{0x0102030405060708ull}));

    auto r = makeReader(buf);
    CHECK(r.u64() == 0x0102030405060708ull);
}

TEST_CASE("ByteWriter / ByteReader: i32 negative", "[reflection][bytestream]")
{
    std::vector<std::byte> buf;
    ByteWriter w(buf, 8);
    REQUIRE(w.write(int32_t{-1}));

    auto r = makeReader(buf);
    CHECK(r.i32() == -1);
}

TEST_CASE("ByteWriter / ByteReader: float round-trip (IEEE 754)", "[reflection][bytestream]")
{
    std::vector<std::byte> buf;
    ByteWriter w(buf, 8);
    REQUIRE(w.write(3.14f));

    auto r = makeReader(buf);
    auto v = r.f32();
    REQUIRE(v.has_value());
    CHECK(*v == Catch::Approx(3.14f));
}

TEST_CASE("ByteWriter / ByteReader: string round-trip", "[reflection][bytestream]")
{
    std::vector<std::byte> buf;
    ByteWriter w(buf, 64);
    REQUIRE(w.write(std::string_view("Sedjro")));

    auto r = makeReader(buf);
    auto s = r.string(256);
    REQUIRE(s.has_value());
    CHECK(*s == "Sedjro");
}

TEST_CASE("ByteReader::string fails when length exceeds maxLength", "[reflection][bytestream]")
{
    std::vector<std::byte> buf;
    ByteWriter w(buf, 64);
    REQUIRE(w.write(std::string_view("hello")));

    auto r = makeReader(buf);
    auto s = r.string(3);  // "hello" is 5 bytes — exceeds limit
    CHECK_FALSE(s.has_value());
    CHECK(r.failed());
}

TEST_CASE("ByteWriter overflows when limit exceeded", "[reflection][bytestream]")
{
    std::vector<std::byte> buf;
    ByteWriter w(buf, 2);
    REQUIRE(w.write(uint8_t{0xAA}));
    REQUIRE(w.write(uint8_t{0xBB}));
    CHECK_FALSE(w.write(uint8_t{0xCC}));
    CHECK(w.overflowed());
    CHECK(buf.size() == 2);
}

TEST_CASE("ByteReader returns nullopt on underrun", "[reflection][bytestream]")
{
    std::vector<std::byte> buf = {std::byte{0x01}};
    auto r = makeReader(buf);
    CHECK_FALSE(r.u16().has_value());
    CHECK(r.failed());
}

TEST_CASE("ByteWriter / ByteReader: multiple values in sequence", "[reflection][bytestream]")
{
    std::vector<std::byte> buf;
    ByteWriter w(buf, 128);
    w.write(uint8_t{1});
    w.write(uint16_t{1000});
    w.write(uint32_t{0xCAFEBABE});
    w.write(std::string_view("rtype"));
    CHECK_FALSE(w.overflowed());

    auto r = makeReader(buf);
    CHECK(r.u8()  == uint8_t{1});
    CHECK(r.u16() == uint16_t{1000});
    CHECK(r.u32() == 0xCAFEBABEu);
    CHECK(r.string(32) == "rtype");
    CHECK(r.remaining() == 0);
}
