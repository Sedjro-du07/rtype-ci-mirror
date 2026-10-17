/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** StreamFramer
*/
// SRV-06 — StreamFramer: TCP message framing with size limit
// Spec: docs/engine/reference/10-network.md · ADR 0037 · RFC §4.3

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Error.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace engine {

/// Cuts a TCP byte stream into discrete messages using a 4-byte big-endian length prefix.
///
/// Wire format per RFC §4.3:
///   - TCP stream starts with a 3-byte preamble (magic 0x5254, version 0x01) — sent once by the
///     caller before any frames.
///   - Each frame: [ length u32 BE ][ id u16 ][ payload[length - 2] ]
///     where length counts id + payload and must be in [2, maxMessageBytes].
///
/// One StreamFramer lives per TCP connection. feed() accumulates raw bytes; next() extracts
/// one complete message at a time.  A protocol violation makes feed() return an error: the
/// caller MUST close the stream immediately.
class ENGINE_API StreamFramer {
public:
    /// @param maxMessageBytes  Upper bound on `length`; matches net.maxStreamMessageBytes (65 536).
    explicit StreamFramer(uint32_t maxMessageBytes);

    /// Append raw bytes received from the TCP stream.
    /// Returns an error (Errc::NetworkError) if the next announced length is out of [2, max_].
    /// The caller MUST close the stream on error and not call feed() or next() again.
    Result<void> feed(std::span<const std::byte> bytes);

    /// Extract the next complete message from the internal buffer.
    /// Returns nullopt when more bytes are needed.
    /// The returned bytes are [ id u16 ][ payload ] (no length prefix).
    std::optional<std::vector<std::byte>> next();

    /// Prepend a 4-byte big-endian length prefix to `message` and append the result to `out`.
    /// `message` must be [ id u16 ][ payload ]; length = message.size().
    /// The caller is responsible for sending the 3-byte preamble once before the first frame.
    static void frame(std::span<const std::byte> message, std::vector<std::byte>& out);

private:
    std::vector<std::byte> buffer_;
    uint32_t max_;
};

} // namespace engine
