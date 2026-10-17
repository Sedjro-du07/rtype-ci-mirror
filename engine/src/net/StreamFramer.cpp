/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** StreamFramer
*/
// SRV-06 — StreamFramer: TCP message framing with size limit
// Spec: docs/engine/reference/10-network.md · ADR 0037 · RFC §4.3

#include "StreamFramer.hpp"

#include <cstring>

namespace engine {

// ---------------------------------------------------------------------------
// Helpers — big-endian 32-bit read without alignment requirements
// ---------------------------------------------------------------------------

static uint32_t readU32BE(const std::byte* p) noexcept
{
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) <<  8) |
           (static_cast<uint32_t>(p[3]));
}

static void writeU32BE(uint32_t v, std::byte* p) noexcept
{
    p[0] = static_cast<std::byte>((v >> 24) & 0xFF);
    p[1] = static_cast<std::byte>((v >> 16) & 0xFF);
    p[2] = static_cast<std::byte>((v >>  8) & 0xFF);
    p[3] = static_cast<std::byte>( v        & 0xFF);
}

// ---------------------------------------------------------------------------
// StreamFramer
// ---------------------------------------------------------------------------

StreamFramer::StreamFramer(uint32_t maxMessageBytes)
    : max_(maxMessageBytes)
{
}

Result<void> StreamFramer::feed(std::span<const std::byte> bytes)
{
    buffer_.insert(buffer_.end(), bytes.begin(), bytes.end());

    // Validate the announced length of the next frame as soon as we have the 4-byte header,
    // before waiting for the payload — RFC §4.3: "MUST close the stream … before reading the
    // payload".
    if (buffer_.size() >= 4) {
        const uint32_t announced = readU32BE(buffer_.data());
        if (announced < 2 || announced > max_) {
            return fail(Errc::NetworkError,
                "StreamFramer: announced frame length " + std::to_string(announced) +
                " is out of valid range [2, " + std::to_string(max_) + "]; closing stream");
        }
    }
    return {};
}

std::optional<std::vector<std::byte>> StreamFramer::next()
{
    // Need at least the 4-byte length header.
    if (buffer_.size() < 4)
        return std::nullopt;

    const uint32_t msgLen = readU32BE(buffer_.data()); // id(2) + payload(msgLen-2)

    // Validity was already checked in feed(); guard here in case next() is called standalone.
    if (msgLen < 2 || msgLen > max_)
        return std::nullopt;

    // Full frame = 4 (length header) + msgLen bytes
    const size_t frameSize = 4 + static_cast<size_t>(msgLen);
    if (buffer_.size() < frameSize)
        return std::nullopt;

    // Extract [ id u16 ][ payload ] — the length prefix is consumed but not returned.
    std::vector<std::byte> msg(buffer_.begin() + 4,
                               buffer_.begin() + static_cast<std::ptrdiff_t>(frameSize));

    buffer_.erase(buffer_.begin(),
                  buffer_.begin() + static_cast<std::ptrdiff_t>(frameSize));

    return msg;
}

void StreamFramer::frame(std::span<const std::byte> message, std::vector<std::byte>& out)
{
    // length field = message.size() (id u16 + payload); message.size() must fit in u32.
    const auto len = static_cast<uint32_t>(message.size());

    std::byte header[4];
    writeU32BE(len, header);

    out.insert(out.end(), header, header + 4);
    out.insert(out.end(), message.begin(), message.end());
}

} // namespace engine
