/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RoomTypesTest
*/
// SRV-08 — Tests for RoomTypes and RoomEvents

#include "engine/net/RoomTypes.hpp"

#include "engine/net/RoomEvents.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace engine;

// ---------------------------------------------------------------------------
// TypeKey macros registered ENGINE_EVENT types
// ---------------------------------------------------------------------------

TEST_CASE("ENGINE_EVENT: PartyChanged has TypeName with Event category", "[room][types]") {
    static_assert(HasTypeName<PartyChanged>);
    static_assert(TypeName<PartyChanged>::category == TypeCategory::Event);
    static_assert(TypeName<PartyChanged>::value == "PartyChanged");
    CHECK(TypeName<PartyChanged>::key != 0);
}

TEST_CASE("ENGINE_EVENT: PlayerEnteredRoom registered correctly", "[room][types]") {
    static_assert(HasTypeName<PlayerEnteredRoom>);
    static_assert(TypeName<PlayerEnteredRoom>::category == TypeCategory::Event);
    CHECK(TypeName<PlayerEnteredRoom>::key != TypeName<PlayerLeftRoom>::key);
}

TEST_CASE("ENGINE_ENUM: LobbyRequest has Enum category", "[room][types]") {
    static_assert(HasTypeName<LobbyRequest>);
    static_assert(TypeName<LobbyRequest>::category == TypeCategory::Enum);
}

// ---------------------------------------------------------------------------
// RoomTypes: Party and RoomInfo
// ---------------------------------------------------------------------------

TEST_CASE("Party: size() returns member count", "[room][types]") {
    Party p;
    p.id = 1;
    p.leader = 42;
    p.members = {42, 7};
    CHECK(p.size() == 2);
}

TEST_CASE("Party: default room is 0 (lobby)", "[room][types]") {
    Party p;
    CHECK(p.room == 0);
}

TEST_CASE("RoomSettings: default maxPlayers is 4 and listed is true", "[room][types]") {
    RoomSettings s;
    s.name = "Battle";
    s.level = "levels/01.yaml";
    CHECK(s.maxPlayers == 4);
    CHECK(s.listed == true);
}

TEST_CASE("RoomInfo: default state is Starting", "[room][types]") {
    RoomInfo info;
    CHECK(info.state == RoomState::Starting);
}

// ---------------------------------------------------------------------------
// DisconnectReason (from NetTypes, needed by PlayerLeftRoom)
// ---------------------------------------------------------------------------

TEST_CASE("DisconnectReason: Quit is distinct from Timeout", "[room][net_types]") {
    CHECK(DisconnectReason::Quit != DisconnectReason::Timeout);
    CHECK(static_cast<uint8_t>(DisconnectReason::Quit) == 0);
}

// ---------------------------------------------------------------------------
// All event types are distinct (no key collisions)
// ---------------------------------------------------------------------------

TEST_CASE("All room event TypeName keys are unique", "[room][types]") {
    const uint64_t keys[] = {
        TypeName<PartyChanged>::key,        TypeName<InviteRequested>::key,   TypeName<InviteAnswered>::key,
        TypeName<LeavePartyRequested>::key, TypeName<RoomListRequested>::key, TypeName<RoomStarted>::key,
        TypeName<RoomFinished>::key,        TypeName<RoomFailed>::key,        TypeName<RoomClosed>::key,
        TypeName<PlayerEnteredRoom>::key,   TypeName<PlayerLeftRoom>::key,    TypeName<PartyUpdated>::key,
        TypeName<InviteReceived>::key,      TypeName<RoomListReceived>::key,  TypeName<EnteredRoom>::key,
        TypeName<ReturnedToLobby>::key,     TypeName<RoomInterrupted>::key,   TypeName<OnlinePlayersChanged>::key,
        TypeName<RequestRefused>::key,
    };
    for (size_t i = 0; i < std::size(keys); ++i)
        for (size_t j = i + 1; j < std::size(keys); ++j)
            CHECK(keys[i] != keys[j]);
}
