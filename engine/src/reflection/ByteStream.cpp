/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ByteStream
*/
// ENG-24 — ByteWriter / ByteReader: bounds-checked, big-endian streams
// Spec: docs/engine/reference/04-reflection.md · ADR 0027 · RFC §5

#include "engine/reflection/ByteStream.hpp"

#include <bit>
#include <cstring>
#include <stdexcept>

namespace engine {

// ---------------------------------------------------------------------------
// ByteWriter
// ---------------------------------------------------------------------------

ByteWriter::ByteWriter(std::vector<std::byte>& out, size_t maxBytes) : out_(out), max_(maxBytes) {}

bool ByteWriter::reserve(size_t n) {
    if (overflow_) return false;
    if (out_.size() + n > max_) {
        overflow_ = true;
        return false;
    }
    return true;
}

void ByteWriter::flushBits() {
    if (bitCount_ > 0) {
        if (reserve(1)) out_.push_back(static_cast<std::byte>(bitBuf_));
        bitBuf_ = 0;
        bitCount_ = 0;
    }
}

bool ByteWriter::write(uint8_t v) {
    flushBits();
    if (!reserve(1)) return false;
    out_.push_back(static_cast<std::byte>(v));
    return true;
}

bool ByteWriter::write(uint16_t v) {
    flushBits();
    if (!reserve(2)) return false;
    out_.push_back(static_cast<std::byte>((v >> 8) & 0xFF));
    out_.push_back(static_cast<std::byte>(v & 0xFF));
    return true;
}

bool ByteWriter::write(uint32_t v) {
    flushBits();
    if (!reserve(4)) return false;
    out_.push_back(static_cast<std::byte>((v >> 24) & 0xFF));
    out_.push_back(static_cast<std::byte>((v >> 16) & 0xFF));
    out_.push_back(static_cast<std::byte>((v >> 8) & 0xFF));
    out_.push_back(static_cast<std::byte>(v & 0xFF));
    return true;
}

bool ByteWriter::write(uint64_t v) {
    flushBits();
    if (!reserve(8)) return false;
    for (int shift = 56; shift >= 0; shift -= 8)
        out_.push_back(static_cast<std::byte>((v >> shift) & 0xFF));
    return true;
}

bool ByteWriter::write(int32_t v) {
    return write(static_cast<uint32_t>(v));
}

bool ByteWriter::write(int64_t v) {
    return write(static_cast<uint64_t>(v));
}

bool ByteWriter::write(float v) {
    uint32_t bits;
    std::memcpy(&bits, &v, 4);
    return write(bits);
}

bool ByteWriter::write(double v) {
    uint64_t bits;
    std::memcpy(&bits, &v, 8);
    return write(bits);
}

bool ByteWriter::write(std::string_view sv) {
    flushBits();
    if (sv.size() > 0xFFFF) {
        overflow_ = true;
        return false;
    }
    if (!write(static_cast<uint16_t>(sv.size()))) return false;
    if (!reserve(sv.size())) return false;
    const auto* p = reinterpret_cast<const std::byte*>(sv.data());
    out_.insert(out_.end(), p, p + sv.size());
    return true;
}

bool ByteWriter::writeBits(uint32_t value, uint8_t bitCount) {
    for (uint8_t i = 0; i < bitCount; ++i) {
        bitBuf_ = static_cast<uint8_t>((bitBuf_ << 1) | ((value >> (bitCount - 1 - i)) & 1));
        ++bitCount_;
        if (bitCount_ == 8) {
            if (!reserve(1)) return false;
            out_.push_back(static_cast<std::byte>(bitBuf_));
            bitBuf_ = 0;
            bitCount_ = 0;
        }
    }
    return !overflow_;
}

size_t ByteWriter::size() const noexcept {
    return out_.size();
}
bool ByteWriter::overflowed() const noexcept {
    return overflow_;
}

// ---------------------------------------------------------------------------
// ByteReader
// ---------------------------------------------------------------------------

ByteReader::ByteReader(std::span<const std::byte> in) : in_(in) {}

bool ByteReader::check(size_t n) {
    if (failed_) return false;
    if (pos_ + n > in_.size()) {
        failed_ = true;
        return false;
    }
    return true;
}

void ByteReader::flushBits() {
    if (bitCount_ > 0) {
        pos_ += 1; // consumed during bit reads; already advanced
        bitBuf_ = 0;
        bitCount_ = 0;
    }
}

std::optional<uint8_t> ByteReader::u8() {
    flushBits();
    if (!check(1)) return std::nullopt;
    return static_cast<uint8_t>(in_[pos_++]);
}

std::optional<uint16_t> ByteReader::u16() {
    flushBits();
    if (!check(2)) return std::nullopt;
    uint16_t v = (static_cast<uint16_t>(in_[pos_]) << 8) | static_cast<uint16_t>(in_[pos_ + 1]);
    pos_ += 2;
    return v;
}

std::optional<uint32_t> ByteReader::u32() {
    flushBits();
    if (!check(4)) return std::nullopt;
    uint32_t v = (static_cast<uint32_t>(in_[pos_]) << 24) | (static_cast<uint32_t>(in_[pos_ + 1]) << 16) |
                 (static_cast<uint32_t>(in_[pos_ + 2]) << 8) | static_cast<uint32_t>(in_[pos_ + 3]);
    pos_ += 4;
    return v;
}

std::optional<uint64_t> ByteReader::u64() {
    flushBits();
    if (!check(8)) return std::nullopt;
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i)
        v = (v << 8) | static_cast<uint64_t>(in_[pos_ + i]);
    pos_ += 8;
    return v;
}

std::optional<int32_t> ByteReader::i32() {
    auto v = u32();
    if (!v) return std::nullopt;
    return static_cast<int32_t>(*v);
}

std::optional<int64_t> ByteReader::i64() {
    auto v = u64();
    if (!v) return std::nullopt;
    return static_cast<int64_t>(*v);
}

std::optional<float> ByteReader::f32() {
    auto raw = u32();
    if (!raw) return std::nullopt;
    float v;
    std::memcpy(&v, &*raw, 4);
    return v;
}

std::optional<double> ByteReader::f64() {
    auto raw = u64();
    if (!raw) return std::nullopt;
    double v;
    std::memcpy(&v, &*raw, 8);
    return v;
}

std::optional<std::span<const std::byte>> ByteReader::bytes(size_t count) {
    flushBits();
    if (!check(count)) return std::nullopt;
    const auto view = in_.subspan(pos_, count);
    pos_ += count;
    return view;
}

std::optional<std::string> ByteReader::string(size_t maxLength) {
    flushBits();
    auto len = u16();
    if (!len) return std::nullopt;
    if (*len > maxLength) {
        failed_ = true;
        return std::nullopt;
    }
    if (!check(*len)) return std::nullopt;
    std::string s(reinterpret_cast<const char*>(in_.data() + pos_), *len);
    pos_ += *len;
    return s;
}

std::optional<uint32_t> ByteReader::bits(uint8_t bitCount) {
    uint32_t result = 0;
    for (uint8_t i = 0; i < bitCount; ++i) {
        if (bitCount_ == 0) {
            if (!check(1)) return std::nullopt;
            bitBuf_ = static_cast<uint8_t>(in_[pos_++]);
            bitCount_ = 8;
        }
        result = (result << 1) | ((bitBuf_ >> 7) & 1);
        bitBuf_ = static_cast<uint8_t>(bitBuf_ << 1);
        --bitCount_;
    }
    return result;
}

size_t ByteReader::remaining() const noexcept {
    if (pos_ >= in_.size()) return 0;
    return in_.size() - pos_;
}

bool ByteReader::failed() const noexcept {
    return failed_;
}

} // namespace engine
