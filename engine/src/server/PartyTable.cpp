/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** PartyTable
*/
// SRV-10 — PartyTable: party lifecycle on the lobby thread
// Spec: docs/engine/reference/14-rooms.md · ADR 0038, 0039

#include "PartyTable.hpp"

#include <algorithm>
#include <stdexcept>

namespace engine {

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

PartyId PartyTable::allocId()
{
    return nextId_++;
}

void PartyTable::dissolve(PartyId id)
{
    parties_.erase(id);
    allIds_.erase(std::remove(allIds_.begin(), allIds_.end(), id), allIds_.end());
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

PartyId PartyTable::createSolo(PlayerId p)
{
    const PartyId id = allocId();
    Party& party     = parties_[id];
    party.id         = id;
    party.leader     = p;
    party.members    = {p};
    party.room       = 0;
    playerParty_[p]  = id;
    allIds_.push_back(id);
    return id;
}

void PartyTable::move(PlayerId p, PartyId to)
{
    const PartyId fromId = of(p);
    if (fromId == to) return;

    Party& src = parties_.at(fromId);
    Party& dst = parties_.at(to);

    // Remove from source
    src.members.erase(
        std::remove(src.members.begin(), src.members.end(), p),
        src.members.end());

    // Promote a new leader in the source if needed
    if (src.leader == p && !src.members.empty())
        src.leader = src.members.front();

    // Add to destination
    dst.members.push_back(p);
    playerParty_[p] = to;

    // Dissolve source if empty
    if (src.members.empty())
        dissolve(fromId);
}

void PartyTable::remove(PlayerId p)
{
    const PartyId id = of(p);
    Party& party     = parties_.at(id);

    party.members.erase(
        std::remove(party.members.begin(), party.members.end(), p),
        party.members.end());
    playerParty_.erase(p);

    if (party.members.empty()) {
        dissolve(id);
        return;
    }

    // Promote next member as leader if the leader left
    if (party.leader == p)
        party.leader = party.members.front();
}

void PartyTable::setLeader(PartyId partyId, PlayerId p)
{
    Party& party = parties_.at(partyId);
    if (std::find(party.members.begin(), party.members.end(), p) == party.members.end())
        throw std::invalid_argument("PartyTable::setLeader: player is not a member");
    party.leader = p;
}

const Party& PartyTable::get(PartyId partyId) const
{
    return parties_.at(partyId);
}

PartyId PartyTable::of(PlayerId p) const
{
    const auto it = playerParty_.find(p);
    if (it == playerParty_.end())
        throw std::out_of_range("PartyTable::of: player not found");
    return it->second;
}

std::span<const PartyId> PartyTable::all() const
{
    return {allIds_.data(), allIds_.size()};
}

} // namespace engine
