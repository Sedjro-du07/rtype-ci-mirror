/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** NetTypes
*/
// ENG-36 — INetwork and network types (minimal subset needed by SRV-08)
// Spec: docs/engine/reference/10-network.md · ADR 0033 · reference 10

#pragma once

#include <cstdint>

namespace engine {

using PlayerId = uint32_t;     ///< What the game sees; 0 = none; one per connected player.
using ConnectionId = uint32_t; ///< Engine-internal: one UDP endpoint + its TCP stream.

/// Reason a player left or was dropped (RFC §6.8).
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

} // namespace engine
