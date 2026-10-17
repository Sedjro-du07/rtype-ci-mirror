/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RoomTypes
*/
// SRV-08 — Room types: PartyId, RoomId, RoomSettings, Party, RoomInfo
// Spec: docs/engine/reference/14-rooms.md · ADR 0038, 0039

#pragma once

#include "engine/net/NetTypes.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace engine {

using PartyId = uint32_t; ///< 0 = none
using RoomId = uint32_t;  ///< 0 = none (player is in the lobby)

/// Lifecycle state of a room (one running game).
enum class RoomState : uint8_t {
    Starting, ///< Created, waiting for all players to send RoomReady
    Running,  ///< All players ready; game systems are ticking
    Finished, ///< reportFinished() called; room still open until closeRoom()
    Failed,   ///< A system threw; room is being torn down
    Closing,  ///< closeRoom() called; parties are being returned to the lobby
};

/// Configuration provided by the game when it calls createRoom().
struct RoomSettings {
    std::string name;  ///< Shown in room lists (RoomList message)
    std::string level; ///< First scene of the room, relative to the game root
    uint32_t maxPlayers = 4;
    bool listed = true; ///< false → private; joined only via moveParty()
};

/// Server-side snapshot of a party.
struct Party {
    PartyId id = 0;
    PlayerId leader = 0;
    std::vector<PlayerId> members; ///< Leader is always included
    RoomId room = 0;               ///< 0 = in the lobby

    size_t size() const { return members.size(); }
};

/// Server-side snapshot of a room as seen from the lobby.
struct RoomInfo {
    RoomId id;
    RoomSettings settings;
    RoomState state = RoomState::Starting;
    std::vector<PartyId> parties;
    std::vector<PlayerId> players;
};

} // namespace engine
