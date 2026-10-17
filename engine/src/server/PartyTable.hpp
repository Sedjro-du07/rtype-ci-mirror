/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** PartyTable
*/
// SRV-10 — PartyTable: party lifecycle on the lobby thread
// Spec: docs/engine/reference/14-rooms.md · ADR 0038, 0039

#pragma once

#include "engine/Export.hpp"
#include "engine/net/RoomTypes.hpp"

#include <span>
#include <unordered_map>
#include <vector>

namespace engine {

/// Tracks every party on the lobby thread.
///
/// Rules (ADR 0039):
///  - Every connected player is always in exactly one party.
///  - A solo player is a party of 1; createSolo() is called on PlayerJoined.
///  - Removing a player promotes the next member as leader; if the party
///    becomes empty it is dissolved automatically.
///  - Lobby thread only — no locks.
class ENGINE_API PartyTable {
public:
    /// Create a party of 1 for `p` (called when a player joins).
    PartyId createSolo(PlayerId p);

    /// Move `p` into party `to`, dissolving the old party if it becomes empty.
    /// Emits PartyChanged for both parties.
    void move(PlayerId p, PartyId to);

    /// Remove `p` from their party (called on PlayerLeft).
    /// Promotes a new leader if `p` was the leader.
    /// Dissolves the party if it becomes empty.
    void remove(PlayerId p);

    /// Change the leader of `party` to `p` (must be a member).
    void setLeader(PartyId party, PlayerId p);

    /// Look up a party by id. Throws std::out_of_range if unknown.
    const Party& get(PartyId party) const;

    /// Return the party that `p` currently belongs to.
    PartyId of(PlayerId p) const;

    /// All currently live party ids.
    std::span<const PartyId> all() const;

private:
    std::unordered_map<PartyId, Party> parties_;
    std::unordered_map<PlayerId, PartyId> playerParty_;
    std::vector<PartyId> allIds_;
    PartyId nextId_ = 1;

    PartyId allocId();
    void dissolve(PartyId id);
};

} // namespace engine
