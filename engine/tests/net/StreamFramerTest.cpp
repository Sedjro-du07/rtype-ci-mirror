/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** StreamFramerTest
*/
// SRV-06 — Tests for StreamFramer
// Done-when: messages split across reads, several in one read, oversized closes the stream.

#include "net/StreamFramer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

using namespace engine;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Build a framed byte sequence: [length u32 BE][payload].
/// payload is treated as id(u16) + body — we pass it verbatim.
static std::vector<std::byte> makeFrame(std::vector<uint8_t> payload)
{
    std::vector<std::byte> out;
    std::span<const std::byte> sp(reinterpret_cast<const std::byte*>(payload.data()), payload.size());
    StreamFramer::frame(sp, out);
    return out;
}

static std::span<const std::byte> toSpan(const std::vector<std::byte>& v)
{
    return {v.data(), v.size()};
}

// ---------------------------------------------------------------------------
// frame() static helper
// ---------------------------------------------------------------------------

TEST_CASE("frame() prepends a 4-byte big-endian length prefix", "[net][stream_framer]")
{
    // message = id(0x0001) + payload "hi"
    std::vector<uint8_t> msg = {0x00, 0x01, 0x68, 0x69}; // id=1, 'h','i'
    auto framed = makeFrame(msg);

    REQUIRE(framed.size() == 8); // 4 (header) + 4 (msg)

    // length field = 4 (msg.size()), big-endian
    CHECK(static_cast<uint8_t>(framed[0]) == 0x00);
    CHECK(static_cast<uint8_t>(framed[1]) == 0x00);
    CHECK(static_cast<uint8_t>(framed[2]) == 0x00);
    CHECK(static_cast<uint8_t>(framed[3]) == 0x04);

    // payload unchanged
    CHECK(static_cast<uint8_t>(framed[4]) == 0x00);
    CHECK(static_cast<uint8_t>(framed[5]) == 0x01);
    CHECK(static_cast<uint8_t>(framed[6]) == 0x68);
    CHECK(static_cast<uint8_t>(framed[7]) == 0x69);
}

// ---------------------------------------------------------------------------
// feed() + next(): happy path
// ---------------------------------------------------------------------------

TEST_CASE("one complete message arrives in one feed()", "[net][stream_framer]")
{
    StreamFramer sf(65536);

    std::vector<uint8_t> msg = {0x00, 0x01, 0xAA, 0xBB};
    auto framed = makeFrame(msg);

    REQUIRE(sf.feed(toSpan(framed)));

    auto result = sf.next();
    REQUIRE(result.has_value());
    CHECK(result->size() == 4);
    CHECK(static_cast<uint8_t>((*result)[0]) == 0x00);
    CHECK(static_cast<uint8_t>((*result)[1]) == 0x01);
    CHECK(static_cast<uint8_t>((*result)[2]) == 0xAA);
    CHECK(static_cast<uint8_t>((*result)[3]) == 0xBB);

    // Buffer exhausted
    CHECK_FALSE(sf.next().has_value());
}

TEST_CASE("several messages arrive concatenated in one feed()", "[net][stream_framer]")
{
    StreamFramer sf(65536);

    std::vector<uint8_t> m1 = {0x00, 0x01, 0x11};
    std::vector<uint8_t> m2 = {0x00, 0x02, 0x22, 0x33};
    std::vector<uint8_t> m3 = {0x00, 0x03};

    std::vector<std::byte> bulk;
    StreamFramer::frame({reinterpret_cast<const std::byte*>(m1.data()), m1.size()}, bulk);
    StreamFramer::frame({reinterpret_cast<const std::byte*>(m2.data()), m2.size()}, bulk);
    StreamFramer::frame({reinterpret_cast<const std::byte*>(m3.data()), m3.size()}, bulk);

    REQUIRE(sf.feed(toSpan(bulk)));

    auto r1 = sf.next();
    REQUIRE(r1.has_value());
    CHECK(r1->size() == 3);

    auto r2 = sf.next();
    REQUIRE(r2.has_value());
    CHECK(r2->size() == 4);

    auto r3 = sf.next();
    REQUIRE(r3.has_value());
    CHECK(r3->size() == 2);

    CHECK_FALSE(sf.next().has_value());
}

TEST_CASE("message split across two feed() calls is reassembled", "[net][stream_framer]")
{
    StreamFramer sf(65536);

    std::vector<uint8_t> msg = {0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF};
    auto framed = makeFrame(msg); // 4 + 6 = 10 bytes

    // Feed first 5 bytes — incomplete
    auto part1 = std::span<const std::byte>(framed.data(), 5);
    REQUIRE(sf.feed(part1));
    CHECK_FALSE(sf.next().has_value());

    // Feed the rest
    auto part2 = std::span<const std::byte>(framed.data() + 5, framed.size() - 5);
    REQUIRE(sf.feed(part2));

    auto result = sf.next();
    REQUIRE(result.has_value());
    CHECK(result->size() == 6);
    CHECK(static_cast<uint8_t>((*result)[2]) == 0xDE);
    CHECK(static_cast<uint8_t>((*result)[3]) == 0xAD);
}

TEST_CASE("message split one byte at a time is reassembled", "[net][stream_framer]")
{
    StreamFramer sf(65536);

    std::vector<uint8_t> msg = {0x00, 0x05, 0x01, 0x02};
    auto framed = makeFrame(msg);

    for (size_t i = 0; i < framed.size() - 1; ++i) {
        REQUIRE(sf.feed({framed.data() + i, 1}));
        CHECK_FALSE(sf.next().has_value());
    }
    REQUIRE(sf.feed({framed.data() + framed.size() - 1, 1}));
    CHECK(sf.next().has_value());
}

// ---------------------------------------------------------------------------
// feed(): protocol errors
// ---------------------------------------------------------------------------

TEST_CASE("announced length above max closes the stream", "[net][stream_framer]")
{
    StreamFramer sf(1024);

    // Craft a frame header announcing 65536 bytes (above limit)
    std::vector<std::byte> bad = {
        std::byte{0x00}, std::byte{0x01}, std::byte{0x00}, std::byte{0x00} // length = 65536
    };
    auto result = sf.feed(toSpan(bad));
    CHECK_FALSE(result); // must return error
}

TEST_CASE("announced length of 0 closes the stream", "[net][stream_framer]")
{
    StreamFramer sf(65536);

    std::vector<std::byte> bad = {
        std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00} // length = 0
    };
    auto result = sf.feed(toSpan(bad));
    CHECK_FALSE(result);
}

TEST_CASE("announced length of 1 closes the stream (must be >= 2)", "[net][stream_framer]")
{
    StreamFramer sf(65536);

    std::vector<std::byte> bad = {
        std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x01} // length = 1
    };
    auto result = sf.feed(toSpan(bad));
    CHECK_FALSE(result);
}

TEST_CASE("announced length exactly equal to max is accepted", "[net][stream_framer]")
{
    const uint32_t maxLen = 128;
    StreamFramer sf(maxLen);

    // Build a message of exactly maxLen bytes (id u16 + 126 bytes payload)
    std::vector<uint8_t> msg(maxLen);
    msg[0] = 0x00; msg[1] = 0x01; // id
    for (size_t i = 2; i < maxLen; ++i) msg[i] = static_cast<uint8_t>(i & 0xFF);

    auto framed = makeFrame(msg);
    REQUIRE(sf.feed(toSpan(framed)));
    auto result = sf.next();
    REQUIRE(result.has_value());
    CHECK(result->size() == maxLen);
}
