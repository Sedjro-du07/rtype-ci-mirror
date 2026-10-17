/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RoomDirectory
*/
// SRV-10 — RoomDirectory: room records on the lobby thread
// Spec: docs/engine/reference/14-rooms.md · ADR 0038, 0039

#include "RoomDirectory.hpp"

#include <algorithm>
#include <stdexcept>

namespace engine {

RoomDirectory::RoomDirectory(uint32_t maxRooms)
    : maxRooms_(maxRooms)
{
}

std::optional<RoomId> RoomDirectory::add(const RoomSettings& settings)
{
    if (rooms_.size() >= maxRooms_)
        return std::nullopt;

    const RoomId id = nextId_++;
    RoomInfo& info  = rooms_[id];
    info.id         = id;
    info.settings   = settings;
    info.state      = RoomState::Starting;
    allIds_.push_back(id);
    return id;
}

void RoomDirectory::remove(RoomId room)
{
    rooms_.erase(room);
    allIds_.erase(std::remove(allIds_.begin(), allIds_.end(), room), allIds_.end());
}

void RoomDirectory::setState(RoomId room, RoomState state)
{
    rooms_.at(room).state = state;
}

void RoomDirectory::enter(RoomId room, const Party& party)
{
    RoomInfo& info = rooms_.at(room);

    if (std::find(info.parties.begin(), info.parties.end(), party.id) == info.parties.end())
        info.parties.push_back(party.id);

    std::vector<PlayerId>& tracked = partyPlayers_[partyKey(room, party.id)];
    for (PlayerId p : party.members) {
        if (std::find(info.players.begin(), info.players.end(), p) == info.players.end())
            info.players.push_back(p);
        if (std::find(tracked.begin(), tracked.end(), p) == tracked.end())
            tracked.push_back(p);
        playerRoom_[p] = room;
    }
}

void RoomDirectory::leave(RoomId room, PartyId partyId)
{
    RoomInfo& info = rooms_.at(room);

    info.parties.erase(
        std::remove(info.parties.begin(), info.parties.end(), partyId),
        info.parties.end());

    const uint64_t key = partyKey(room, partyId);
    const auto it = partyPlayers_.find(key);
    if (it != partyPlayers_.end()) {
        for (PlayerId p : it->second) {
            playerRoom_.erase(p);
            info.players.erase(
                std::remove(info.players.begin(), info.players.end(), p),
                info.players.end());
        }
        partyPlayers_.erase(it);
    }
}

const RoomInfo& RoomDirectory::get(RoomId room) const
{
    return rooms_.at(room);
}

RoomId RoomDirectory::roomOf(PlayerId player) const
{
    const auto it = playerRoom_.find(player);
    return it == playerRoom_.end() ? RoomId{0} : it->second;
}

std::span<const RoomId> RoomDirectory::all() const
{
    return {allIds_.data(), allIds_.size()};
}

} // namespace engine
