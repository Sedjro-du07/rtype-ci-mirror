/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RoomDirectory
*/
// SRV-10 — RoomDirectory: room records on the lobby thread
// Spec: docs/engine/reference/14-rooms.md · ADR 0038, 0039

#pragma once

#include "engine/net/RoomTypes.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

namespace engine {

/// Tracks every room record on the lobby thread.
///
/// Rules (ADR 0038/0039):
///  - add() refuses when the live room count reaches maxRooms.
///  - enter() records the party members in the room; leave() removes them.
///  - roomOf(player) returns 0 when the player is in the lobby.
///  - Lobby thread only — no locks.
class RoomDirectory {
public:
    explicit RoomDirectory(uint32_t maxRooms);

    /// Register a new room; returns its id, or nullopt when maxRooms is reached.
    std::optional<RoomId> add(const RoomSettings& settings);

    /// Remove a room record.
    void remove(RoomId room);

    /// Update the state of a room.
    void setState(RoomId room, RoomState state);

    /// Move an entire party into a room.
    void enter(RoomId room, const Party& party);

    /// Remove a party from a room (clears its players' room tracking).
    void leave(RoomId room, PartyId partyId);

    /// Look up a room by id. Throws std::out_of_range if unknown.
    const RoomInfo& get(RoomId room) const;

    /// Return the RoomId of the room the player is in, or 0 if in the lobby.
    RoomId roomOf(PlayerId player) const;

    /// All currently live room ids.
    std::span<const RoomId> all() const;

private:
    uint32_t                                         maxRooms_;
    std::unordered_map<RoomId, RoomInfo>             rooms_;
    std::unordered_map<PlayerId, RoomId>             playerRoom_;
    /// Maps (roomId, partyId) → list of players that entered with that party.
    std::unordered_map<uint64_t, std::vector<PlayerId>> partyPlayers_;
    std::vector<RoomId>                              allIds_;
    RoomId                                           nextId_ = 1;

    static uint64_t partyKey(RoomId r, PartyId p) noexcept
    {
        return (static_cast<uint64_t>(r) << 32) | p;
    }
};

} // namespace engine
