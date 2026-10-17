/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** NetTypes
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Error.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace engine {

using PlayerId = uint32_t;
using ConnectionId = uint32_t;

enum class DisconnectReason : uint8_t {
    Quit,
    Timeout,
    Kicked,
    ProtocolError,
    ServerShutdown,
    ServerFull,
    FormatMismatch,
    ProtocolMismatch,
    WrongGame,
    InvalidName,
    RefusedByGame,
    ConnectionLost,
};

struct ENGINE_API Endpoint {
    std::array<uint8_t, 16> address{};
    uint16_t port = 0;
    bool v6 = false;

    std::string toString() const;
    static Result<Endpoint> resolve(std::string_view host, uint16_t port);
    static Endpoint ipv4(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port);
    bool operator==(const Endpoint&) const = default;
};

struct Datagram {
    Endpoint from;
    std::vector<std::byte> bytes;
};

struct NetConfig {
    uint16_t port = 4242;
    uint32_t maxClients = 32;
    double heartbeatSeconds = 0.5;
    double timeoutSeconds = 5.0;
    uint32_t maxDatagramBytes = 1200;
    uint32_t maxQueuedDatagrams = 1024;
    double snapshotRate = 20.0;
    uint32_t maxNameLength = 16;
    uint32_t maxStreamMessageBytes = 65536;
    double handshakeSeconds = 5.0;
    uint32_t maxPendingHandshakes = 16;
    double reconnectGraceSeconds = 30.0;
    bool autoReconnect = true;
    uint32_t sendBudgetBytesPerSecond = 65536;
    double interpolationDelay = 0.1;
    uint32_t inputRedundancy = 3;
    bool shareOnlinePlayers = true;
};

struct ConnectionStats {
    double rttMs = 0;
    double lossPercent = 0;
    uint64_t bytesIn = 0, bytesOut = 0;
    uint64_t packetsIn = 0, packetsOut = 0, dropped = 0;
};

}

template <>
struct ENGINE_API std::hash<engine::Endpoint> {
    size_t operator()(const engine::Endpoint& endpoint) const noexcept;
};

ENGINE_ENUM(engine::DisconnectReason, Quit, Timeout, Kicked, ProtocolError, ServerShutdown, ServerFull, FormatMismatch,
            ProtocolMismatch, WrongGame, InvalidName, RefusedByGame, ConnectionLost)
