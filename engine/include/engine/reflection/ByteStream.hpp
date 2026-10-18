/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ByteStream
*/
// ENG-24 — ByteWriter / ByteReader: bounds-checked, big-endian streams
// Spec: docs/engine/reference/04-reflection.md · ADR 0027 · RFC §5

#pragma once

#include "engine/Export.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace engine {

/// Appends big-endian encoded values to a caller-owned buffer.
/// Every write returns false and sets overflowed() if adding the bytes would exceed maxBytes.
/// Floats are written as their IEEE 754 bit pattern (RFC §5).
class ENGINE_API ByteWriter {
public:
    explicit ByteWriter(std::vector<std::byte>& out, size_t maxBytes);

    bool write(uint8_t v);
    bool write(uint16_t v);
    bool write(uint32_t v);
    bool write(uint64_t v);
    bool write(int32_t v);
    bool write(int64_t v);
    bool write(float v);  // IEEE 754, big-endian bit pattern
    bool write(double v); // IEEE 754, big-endian bit pattern
    /// Writes a length-prefixed UTF-8 string: u16 byte count then the bytes (RFC §5).
    bool write(std::string_view sv);
    /// Bit-packing: writes the lowest bitCount bits of value.
    bool writeBits(uint32_t value, uint8_t bitCount);

    size_t size() const noexcept;
    bool overflowed() const noexcept;

private:
    std::vector<std::byte>& out_;
    size_t max_;
    bool overflow_ = false;
    uint8_t bitBuf_ = 0;
    uint8_t bitCount_ = 0;

    bool reserve(size_t n);
    void flushBits();
};

/// Bounds-checked, big-endian reader over a contiguous byte span.
/// Every read returns std::nullopt (and sets failed()) when not enough bytes remain.
class ENGINE_API ByteReader {
public:
    explicit ByteReader(std::span<const std::byte> in);

    std::optional<uint8_t> u8();
    std::optional<uint16_t> u16();
    std::optional<uint32_t> u32();
    std::optional<uint64_t> u64();
    std::optional<int32_t> i32();
    std::optional<int64_t> i64();
    std::optional<float> f32();  // IEEE 754 big-endian
    std::optional<double> f64(); // IEEE 754 big-endian
    /// Reads a u16-length-prefixed string; fails if length > maxLength or bytes missing.
    std::optional<std::string> string(size_t maxLength);
    /// Reads count raw bytes and returns a view into the input span.
    std::optional<std::span<const std::byte>> bytes(size_t count);
    /// Bit-packing: reads bitCount bits into the low bits of the result.
    std::optional<uint32_t> bits(uint8_t bitCount);

    size_t remaining() const noexcept;
    bool failed() const noexcept;

private:
    std::span<const std::byte> in_;
    size_t pos_ = 0;
    bool failed_ = false;
    uint8_t bitBuf_ = 0;
    uint8_t bitCount_ = 0;

    bool check(size_t n);
    void flushBits();
};

} // namespace engine
