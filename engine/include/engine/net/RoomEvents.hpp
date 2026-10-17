/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RoomEvents
*/
// SRV-08 — Room events: lobby, room and client events
// Spec: docs/engine/reference/14-rooms.md · ADR 0038, 0039

#pragma once

#include "engine/net/RoomTypes.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace engine {

// ---------------------------------------------------------------------------
// Lobby events (server side)
// Published on the lobby EventBus; received via LobbyContext.
// ---------------------------------------------------------------------------

/// A party's composition or room changed; re-read PartyTable::get(party).
struct PartyChanged {
    PartyId party;
};

/// A player sent an invite to another player.
struct InviteRequested {
    PlayerId from;
    PlayerId to;
};

/// A player answered an invite.
struct InviteAnswered {
    PlayerId player;
    PartyId party;
    bool accepted;
};

/// A player asked to leave their party (back to a party of 1).
struct LeavePartyRequested {
    PlayerId player;
};

/// A player requested the list of joinable rooms.
struct RoomListRequested {
    PlayerId player;
};

/// A room transitioned to RoomState::Running (all players sent RoomReady).
struct RoomStarted {
    RoomId room;
};

/// The game called reportFinished() in a room.
struct RoomFinished {
    RoomId room;
    std::vector<std::byte> result; ///< Encoded result; decode with resultAs<T>().

    template <class T>
    std::optional<T> resultAs() const;
};

/// A system threw inside a room; the room is closing.
struct RoomFailed {
    RoomId room;
    std::string error;
};

/// A room finished closing; all parties have been returned to the lobby.
struct RoomClosed {
    RoomId room;
};

// ---------------------------------------------------------------------------
// Room events (in-room, server side)
// Published on the room's own EventBus; received by game systems via Context.
// ---------------------------------------------------------------------------

/// A player (and their party) just entered this room.
struct PlayerEnteredRoom {
    PlayerId player;
    PartyId party;
};

/// A player left (disconnect, kick, Bye, or room close).
struct PlayerLeftRoom {
    PlayerId player;
    DisconnectReason reason;
};

// ---------------------------------------------------------------------------
// Client-side events
// Published on the client's EventBus by the engine when messages arrive.
// ---------------------------------------------------------------------------

/// The server updated the client's party (members, leader, room).
struct PartyUpdated {
    Party party;
};

/// An invite from another player arrived.
struct InviteReceived {
    PlayerId from;
    PartyId party;
};

/// The server answered a RoomListRequest.
struct RoomListReceived {
    std::vector<RoomInfo> rooms;
};

/// The client entered a room (change scene to settings.level).
struct EnteredRoom {
    RoomId room;
    RoomSettings settings;
};

/// The client was returned to the lobby (game over, kicked from room, or manual leave).
struct ReturnedToLobby {
    RoomId room;
    std::string reason;
};

/// The room failed (a system threw); client should return to the lobby UI.
struct RoomInterrupted {
    RoomId room;
};

/// The online player count changed; the client may request an updated list.
struct OnlinePlayersChanged {};

/// A lobby request was refused by the server.
enum class LobbyRequest : uint8_t {
    Invite = 1,
    InviteAnswer,
    LeaveParty,
    RoomList,
    Game,
};

struct RequestRefused {
    LobbyRequest request;
    std::string reason;
};

// ---------------------------------------------------------------------------
// RoomFinished::resultAs<T>() — inline template body
// ---------------------------------------------------------------------------

template <class T>
std::optional<T> RoomFinished::resultAs() const {
    // Decoding requires the Codec (SRV-04) and a TypeInfo for T.
    // Placeholder: returns nullopt until the codec integration is wired in.
    (void)result;
    return std::nullopt;
}

} // namespace engine

ENGINE_EVENT(engine::PartyChanged)
ENGINE_EVENT(engine::InviteRequested)
ENGINE_EVENT(engine::InviteAnswered)
ENGINE_EVENT(engine::LeavePartyRequested)
ENGINE_EVENT(engine::RoomListRequested)
ENGINE_EVENT(engine::RoomStarted)
ENGINE_EVENT(engine::RoomFinished)
ENGINE_EVENT(engine::RoomFailed)
ENGINE_EVENT(engine::RoomClosed)
ENGINE_EVENT(engine::PlayerEnteredRoom)
ENGINE_EVENT(engine::PlayerLeftRoom)
ENGINE_EVENT(engine::PartyUpdated)
ENGINE_EVENT(engine::InviteReceived)
ENGINE_EVENT(engine::RoomListReceived)
ENGINE_EVENT(engine::EnteredRoom)
ENGINE_EVENT(engine::ReturnedToLobby)
ENGINE_EVENT(engine::RoomInterrupted)
ENGINE_EVENT(engine::OnlinePlayersChanged)
ENGINE_EVENT(engine::RequestRefused)
ENGINE_ENUM(engine::LobbyRequest, Invite, InviteAnswer, LeaveParty, RoomList, Game)
