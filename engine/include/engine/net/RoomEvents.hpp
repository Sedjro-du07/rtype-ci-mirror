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
struct PartyChanged { PartyId party; };
ENGINE_EVENT(PartyChanged)

/// A player sent an invite to another player.
struct InviteRequested { PlayerId from; PlayerId to; };
ENGINE_EVENT(InviteRequested)

/// A player answered an invite.
struct InviteAnswered { PlayerId player; PartyId party; bool accepted; };
ENGINE_EVENT(InviteAnswered)

/// A player asked to leave their party (back to a party of 1).
struct LeavePartyRequested { PlayerId player; };
ENGINE_EVENT(LeavePartyRequested)

/// A player requested the list of joinable rooms.
struct RoomListRequested { PlayerId player; };
ENGINE_EVENT(RoomListRequested)

/// A room transitioned to RoomState::Running (all players sent RoomReady).
struct RoomStarted { RoomId room; };
ENGINE_EVENT(RoomStarted)

/// The game called reportFinished() in a room.
struct RoomFinished {
    RoomId room;
    std::vector<std::byte> result;  ///< Encoded result; decode with resultAs<T>().

    template <class T>
    std::optional<T> resultAs() const;
};
ENGINE_EVENT(RoomFinished)

/// A system threw inside a room; the room is closing.
struct RoomFailed { RoomId room; std::string error; };
ENGINE_EVENT(RoomFailed)

/// A room finished closing; all parties have been returned to the lobby.
struct RoomClosed { RoomId room; };
ENGINE_EVENT(RoomClosed)

// ---------------------------------------------------------------------------
// Room events (in-room, server side)
// Published on the room's own EventBus; received by game systems via Context.
// ---------------------------------------------------------------------------

/// A player (and their party) just entered this room.
struct PlayerEnteredRoom { PlayerId player; PartyId party; };
ENGINE_EVENT(PlayerEnteredRoom)

/// A player left (disconnect, kick, Bye, or room close).
struct PlayerLeftRoom { PlayerId player; DisconnectReason reason; };
ENGINE_EVENT(PlayerLeftRoom)

// ---------------------------------------------------------------------------
// Client-side events
// Published on the client's EventBus by the engine when messages arrive.
// ---------------------------------------------------------------------------

/// The server updated the client's party (members, leader, room).
struct PartyUpdated { Party party; };
ENGINE_EVENT(PartyUpdated)

/// An invite from another player arrived.
struct InviteReceived { PlayerId from; PartyId party; };
ENGINE_EVENT(InviteReceived)

/// The server answered a RoomListRequest.
struct RoomListReceived { std::vector<RoomInfo> rooms; };
ENGINE_EVENT(RoomListReceived)

/// The client entered a room (change scene to settings.level).
struct EnteredRoom { RoomId room; RoomSettings settings; };
ENGINE_EVENT(EnteredRoom)

/// The client was returned to the lobby (game over, kicked from room, or manual leave).
struct ReturnedToLobby { RoomId room; std::string reason; };
ENGINE_EVENT(ReturnedToLobby)

/// The room failed (a system threw); client should return to the lobby UI.
struct RoomInterrupted { RoomId room; };
ENGINE_EVENT(RoomInterrupted)

/// The online player count changed; the client may request an updated list.
struct OnlinePlayersChanged {};
ENGINE_EVENT(OnlinePlayersChanged)

/// A lobby request was refused by the server.
enum class LobbyRequest : uint8_t {
    Invite      = 1,
    InviteAnswer,
    LeaveParty,
    RoomList,
    Game,
};
ENGINE_ENUM(LobbyRequest, LobbyRequest::Invite, LobbyRequest::InviteAnswer,
            LobbyRequest::LeaveParty, LobbyRequest::RoomList, LobbyRequest::Game)

struct RequestRefused { LobbyRequest request; std::string reason; };
ENGINE_EVENT(RequestRefused)

// ---------------------------------------------------------------------------
// RoomFinished::resultAs<T>() — inline template body
// ---------------------------------------------------------------------------

template <class T>
std::optional<T> RoomFinished::resultAs() const
{
    // Decoding requires the Codec (SRV-04) and a TypeInfo for T.
    // Placeholder: returns nullopt until the codec integration is wired in.
    (void)result;
    return std::nullopt;
}

} // namespace engine
