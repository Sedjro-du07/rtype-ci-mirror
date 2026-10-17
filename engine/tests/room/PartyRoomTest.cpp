/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** PartyRoomTest
*/
// SRV-10 — Tests for PartyTable and RoomDirectory
// Done-when: join → party of 1; invite + accept → one party of 2 and the old dissolved;
//            leader leaves → new leader; maxRooms respected.

#include "server/PartyTable.hpp"
#include "server/RoomDirectory.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace engine;

// ---------------------------------------------------------------------------
// PartyTable tests
// ---------------------------------------------------------------------------

TEST_CASE("PartyTable: createSolo creates a party of 1", "[party]") {
    PartyTable pt;
    const PartyId pid = pt.createSolo(42);
    const Party& p = pt.get(pid);
    CHECK(p.id == pid);
    CHECK(p.leader == 42);
    CHECK(p.size() == 1);
    CHECK(p.members[0] == 42);
    CHECK(pt.of(42) == pid);
}

TEST_CASE("PartyTable: two solos, then move one into the other", "[party]") {
    PartyTable pt;
    const PartyId p1 = pt.createSolo(1);
    const PartyId p2 = pt.createSolo(2);

    pt.move(2, p1);

    const Party& party = pt.get(p1);
    CHECK(party.size() == 2);
    CHECK(pt.of(2) == p1);

    // p2 is dissolved (was solo)
    CHECK_THROWS(pt.get(p2));
    CHECK(pt.all().size() == 1);
}

TEST_CASE("PartyTable: removing the leader promotes next member", "[party]") {
    PartyTable pt;
    const PartyId pid = pt.createSolo(10);
    pt.createSolo(20);
    pt.move(20, pid);

    // Party now has {10(leader), 20}
    pt.remove(10);

    const Party& party = pt.get(pid);
    CHECK(party.leader == 20);
    CHECK(party.size() == 1);
}

TEST_CASE("PartyTable: removing last member dissolves the party", "[party]") {
    PartyTable pt;
    const PartyId pid = pt.createSolo(7);
    pt.remove(7);

    CHECK(pt.all().empty());
    CHECK_THROWS(pt.get(pid));
}

TEST_CASE("PartyTable: setLeader changes leader", "[party]") {
    PartyTable pt;
    const PartyId pid = pt.createSolo(1);
    pt.createSolo(2);
    pt.move(2, pid);

    pt.setLeader(pid, 2);
    CHECK(pt.get(pid).leader == 2);
}

TEST_CASE("PartyTable: setLeader with non-member throws", "[party]") {
    PartyTable pt;
    const PartyId pid = pt.createSolo(1);
    CHECK_THROWS(pt.setLeader(pid, 99));
}

TEST_CASE("PartyTable: all() lists all live parties", "[party]") {
    PartyTable pt;
    pt.createSolo(1);
    pt.createSolo(2);
    pt.createSolo(3);
    CHECK(pt.all().size() == 3);
}

TEST_CASE("PartyTable: move within same party is a no-op", "[party]") {
    PartyTable pt;
    const PartyId pid = pt.createSolo(5);
    pt.move(5, pid);
    CHECK(pt.get(pid).size() == 1);
}

// ---------------------------------------------------------------------------
// RoomDirectory tests
// ---------------------------------------------------------------------------

static Party makeParty(PartyId id, PlayerId leader, std::vector<PlayerId> members) {
    Party p;
    p.id = id;
    p.leader = leader;
    p.members = std::move(members);
    p.room = 0;
    return p;
}

TEST_CASE("RoomDirectory: add creates a room in Starting state", "[room]") {
    RoomDirectory rd(8);
    RoomSettings s;
    s.name = "Battle";
    s.level = "levels/01.yaml";

    const auto id = rd.add(s);
    REQUIRE(id.has_value());
    CHECK(rd.get(*id).state == RoomState::Starting);
    CHECK(rd.get(*id).settings.name == "Battle");
    CHECK(rd.all().size() == 1);
}

TEST_CASE("RoomDirectory: add returns nullopt when maxRooms is reached", "[room]") {
    RoomDirectory rd(2);
    RoomSettings s;
    REQUIRE(rd.add(s).has_value());
    REQUIRE(rd.add(s).has_value());
    CHECK_FALSE(rd.add(s).has_value());
}

TEST_CASE("RoomDirectory: remove erases the room", "[room]") {
    RoomDirectory rd(8);
    RoomSettings s;
    const RoomId id = *rd.add(s);
    rd.remove(id);
    CHECK(rd.all().empty());
    CHECK_THROWS(rd.get(id));
}

TEST_CASE("RoomDirectory: setState changes the room's state", "[room]") {
    RoomDirectory rd(8);
    const RoomId id = *rd.add({});
    rd.setState(id, RoomState::Running);
    CHECK(rd.get(id).state == RoomState::Running);
}

TEST_CASE("RoomDirectory: enter adds party and players", "[room]") {
    RoomDirectory rd(8);
    const RoomId rid = *rd.add({});
    const Party p = makeParty(1, 10, {10, 20});

    rd.enter(rid, p);

    const RoomInfo& info = rd.get(rid);
    CHECK(info.parties.size() == 1);
    CHECK(info.players.size() == 2);
    CHECK(rd.roomOf(10) == rid);
    CHECK(rd.roomOf(20) == rid);
}

TEST_CASE("RoomDirectory: leave removes party and players", "[room]") {
    RoomDirectory rd(8);
    const RoomId rid = *rd.add({});
    const Party p = makeParty(1, 10, {10, 20});

    rd.enter(rid, p);
    rd.leave(rid, 1);

    const RoomInfo& info = rd.get(rid);
    CHECK(info.parties.empty());
    CHECK(info.players.empty());
    CHECK(rd.roomOf(10) == 0);
    CHECK(rd.roomOf(20) == 0);
}

TEST_CASE("RoomDirectory: roomOf returns 0 for players in the lobby", "[room]") {
    RoomDirectory rd(8);
    CHECK(rd.roomOf(999) == 0);
}

TEST_CASE("RoomDirectory: two parties in same room, leave one, keep the other", "[room]") {
    RoomDirectory rd(8);
    const RoomId rid = *rd.add({});
    const Party p1 = makeParty(1, 10, {10});
    const Party p2 = makeParty(2, 20, {20, 30});

    rd.enter(rid, p1);
    rd.enter(rid, p2);
    rd.leave(rid, 1);

    const RoomInfo& info = rd.get(rid);
    CHECK(info.parties.size() == 1);
    CHECK(info.players.size() == 2);
    CHECK(rd.roomOf(10) == 0);
    CHECK(rd.roomOf(20) == rid);
    CHECK(rd.roomOf(30) == rid);
}

TEST_CASE("RoomDirectory: after remove, a new room can be added", "[room]") {
    RoomDirectory rd(1);
    const RoomId id = *rd.add({});
    rd.remove(id);
    const auto id2 = rd.add({});
    CHECK(id2.has_value());
}
