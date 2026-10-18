/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ReplicationTest
*/

#include "net/Replication.hpp"

#include "engine/core/Time.hpp"
#include "engine/events/EventBus.hpp"
#include "engine/input/InputMap.hpp"
#include "engine/net/NetComponents.hpp"
#include "engine/net/NetworkIdMap.hpp"
#include "engine/net/NetworkSession.hpp"
#include "engine/reflection/ByteStream.hpp"
#include "engine/scene/Prefab.hpp"
#include "scene/SceneFixture.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace repltest {

struct Shield {
    int32_t value = 0;
};

struct Target {
    engine::Entity entity;
};

}

ENGINE_COMPONENT(repltest::Shield)
ENGINE_COMPONENT(repltest::Target)

using namespace engine;
using namespace repltest;
using Catch::Approx;
using scenetest::Health;

namespace {

constexpr ActionId Fire = action("fire");
constexpr ActionId Up = action("up");
constexpr uint32_t SessionKey = 0x5EED1234;
constexpr PlayerId FirstPlayer = 7;

std::string unique(const std::string& folder) {
    static const std::string suffix = std::to_string(std::random_device{}());
    return folder + "-" + suffix;
}

class HubNetwork;

struct Hub {
    std::unordered_map<Endpoint, HubNetwork*> nodes;
};

class HubNetwork final : public INetwork {
public:
    Hub* hub;
    Endpoint self;
    std::move_only_function<void(Datagram&&)> receiver;

    HubNetwork(Hub& hub, Endpoint self) : hub(&hub), self(self) { hub.nodes[self] = this; }
    ~HubNetwork() override { this->hub->nodes.erase(this->self); }

    PluginKind kind() const override { return PluginKind::Network; }
    std::string_view name() const override { return "hub"; }
    uint32_t apiVersion() const override { return 1; }
    Result<void> init(const PluginContext&) override { return {}; }
    void shutdown() noexcept override {}
    Result<void> openUdp(uint16_t) override { return {}; }
    void closeUdp() override {}
    void sendUdp(const Endpoint& to, std::span<const std::byte> bytes) override {
        const auto found = this->hub->nodes.find(to);
        if (found == this->hub->nodes.end() || !found->second->receiver) return;
        found->second->receiver(Datagram{this->self, std::vector<std::byte>(bytes.begin(), bytes.end())});
    }
    void setReceiver(std::move_only_function<void(Datagram&&)> fn) override { this->receiver = std::move(fn); }
    Result<void> listenTcp(uint16_t) override { return {}; }
    Result<uint32_t> connectTcp(const Endpoint&) override { return 1u; }
    void sendTcp(uint32_t, std::span<const std::byte>) override {}
    void setTcpReceiver(std::move_only_function<void(uint32_t, std::span<const std::byte>)>) override {}
    void closeTcp(uint32_t) override {}
};

struct Peer : scenetest::SceneFixture {
    EventBus events;
    NetConfig config;
    HubNetwork network;
    InputMap input;
    PrefabLibrary prefabs{types, assets};
    std::optional<Registry> world;
    std::optional<NetworkSession> session;
    std::optional<Replication> replication;
    Time time;

    Peer(const std::string& folder, Hub& hub, Endpoint self, bool server)
        : SceneFixture(unique(folder)), network(hub, self) {
        this->types.add<Shield>().field("value", &Shield::value).flags(Replicated);
        this->types.add<Target>().field("entity", &Target::entity).flags(Replicated);
        this->world.emplace(this->types);
        this->write("prefabs/ship.yaml", "NetPrefab: {}\n"
                                         "Sprite: { texture: assets/ship.png, layer: 30 }\n"
                                         "Health: { hp: 3, maxHp: 3 }\n");
        this->write("prefabs/decor.yaml", "Sprite: { texture: assets/ship.png, layer: 1 }\n");
        this->input.name(Fire, "fire");
        this->input.name(Up, "up");
        this->config.snapshotRate = 0;
        this->time.fixedDelta = 1.0 / 60.0;
        this->session.emplace(this->events, this->log, this->config, this->types);
        this->session->attach(this->network);
        REQUIRE(this->session->open(server, server ? 4242 : 0));
        this->replication.emplace(
            ReplicationParts{*this->session, this->types, this->input, &this->prefabs, &this->assets}, this->config,
            this->log);
        this->session->onMessage([this](const InboundMessage& message) { this->replication->receive(message); });
        REQUIRE(this->prefabs.preload("prefabs"));
    }

    Registry& w() { return *this->world; }

    Entity spawn(const std::string& prefab, Vec2 at) {
        const auto found = this->prefabs.get(prefab);
        REQUIRE(found);
        return this->prefabs.instantiate(this->w(), **found, at);
    }

    Entity mirror(const Peer& server, Entity e) {
        return this->replication->ids().find(server.world->get<NetworkId>(e).value);
    }

    size_t networked() { return this->w().view<NetworkId>().sizeHint(); }

    void advance() {
        ++this->time.tick;
        this->time.total += this->time.fixedDelta;
        this->session->poll(this->time.total);
    }

    void receiveSnapshot(std::span<const std::byte> payload) {
        this->replication->receive(InboundMessage{1, SnapshotMessage, std::nullopt, payload});
    }
};

struct Network {
    Hub hub;
    const Endpoint serverAt = Endpoint::ipv4(10, 0, 0, 1, 4242);
    const Endpoint firstAt = Endpoint::ipv4(10, 0, 0, 2, 5000);
    const Endpoint secondAt = Endpoint::ipv4(10, 0, 0, 3, 5000);
    Peer server{"rtype-repl-server", hub, serverAt, true};
    Peer first{"rtype-repl-first", hub, firstAt, false};
    Peer second{"rtype-repl-second", hub, secondAt, false};
    ActionState firstInput;

    Network() {
        this->server.session->openConnection(this->firstAt, SessionKey, FirstPlayer);
        this->server.session->openConnection(this->secondAt, SessionKey + 1, FirstPlayer + 1);
        this->first.session->openConnection(this->serverAt, SessionKey, 0);
        this->second.session->openConnection(this->serverAt, SessionKey + 1, 0);
    }

    void step() {
        this->server.advance();
        this->server.replication->applyInputs(this->server.w());
        this->server.replication->assignIds(this->server.w());
        this->server.replication->sendSnapshots(this->server.w(), this->server.time);
        this->server.session->flush(this->server.time.total);
        for (Peer* client : {&this->first, &this->second}) {
            client->advance();
            client->replication->applySnapshots(client->w(), client->time);
            client->replication->sendInput(client == &this->first ? this->firstInput : ActionState{});
            client->session->flush(client->time.total);
        }
    }

    void run(int steps) {
        for (int i = 0; i < steps; ++i)
            this->step();
    }
};

struct SnapshotWriter {
    std::vector<std::byte> bytes;
    ByteWriter out{bytes, 1024};

    SnapshotWriter(uint16_t roomSession, uint32_t tick, uint16_t count) {
        this->out.write(roomSession);
        this->out.write(tick);
        this->out.write(uint32_t{0});
        this->out.write(count);
    }

    SnapshotWriter& spawn(uint32_t id, uint32_t prefabKey, std::optional<Vec2> at = std::nullopt) {
        return this->entry(id, 1, prefabKey, at);
    }

    SnapshotWriter& update(uint32_t id, Vec2 at) { return this->entry(id, 0, std::nullopt, at); }

    SnapshotWriter& destroy(uint32_t id) {
        this->out.write(id);
        this->out.write(uint8_t{2});
        return *this;
    }

    SnapshotWriter& entry(uint32_t id, uint8_t op, std::optional<uint32_t> prefabKey, std::optional<Vec2> at) {
        this->out.write(id);
        this->out.write(op);
        if (prefabKey) this->out.write(*prefabKey);
        this->out.write(static_cast<uint8_t>(at ? 1 : 0));
        if (!at) return *this;
        this->out.write(uint16_t{0});
        this->out.write(uint16_t{8});
        this->out.write(at->x);
        this->out.write(at->y);
        return *this;
    }
};

}

TEST_CASE("NetworkIdMap hands out ids from 1 and never reuses them", "[net][replication]") {
    NetworkIdMap ids;
    const uint32_t a = ids.allocate();
    const uint32_t b = ids.allocate();
    CHECK(a == 1);
    CHECK(b == 2);
    ids.bind(a, Entity{4, 0});
    CHECK(ids.find(a) == Entity{4, 0});
    CHECK(ids.find(99) == Entity::null());
    ids.unbind(a);
    CHECK(ids.find(a) == Entity::null());
    ids.clear();
    CHECK(ids.allocate() == 3);
}

TEST_CASE("replicated components are listed in registration order, engine first", "[net][replication]") {
    Hub hub;
    Peer peer{"rtype-repl-order", hub, Endpoint::ipv4(10, 0, 0, 9, 1), true};
    std::vector<std::string> names;
    for (const TypeInfo* type : peer.replication->replicated())
        names.push_back(type->name);
    CHECK(names == std::vector<std::string>{"Position", "Velocity", "Sprite", "NetOwner", "Shield", "Target"});
}

TEST_CASE("a networked prefab carries its key and the client preloads it", "[net][replication]") {
    Hub hub;
    Peer peer{"rtype-repl-prefab", hub, Endpoint::ipv4(10, 0, 0, 9, 2), true};
    const Entity ship = peer.spawn("prefabs/ship.yaml", {});
    const Entity decor = peer.spawn("prefabs/decor.yaml", {});
    CHECK(peer.w().get<NetPrefab>(ship).prefabKey == prefabKey("prefabs/ship.yaml"));
    CHECK_FALSE(peer.w().has<NetPrefab>(decor));
    CHECK(peer.prefabs.findByKey(prefabKey("prefabs/decor.yaml")) != nullptr);
}

TEST_CASE("two headless clients see the entities the server spawns", "[net][replication]") {
    Network net;
    const Entity a = net.server.spawn("prefabs/ship.yaml", {10, 20});
    const Entity b = net.server.spawn("prefabs/ship.yaml", {300, 40});
    net.server.spawn("prefabs/decor.yaml", {0, 0});
    net.server.w().emplace<Shield>(a, Shield{5});
    net.server.w().emplace<Target>(a, Target{b});
    net.run(3);

    CHECK(net.server.networked() == 2);
    for (Peer* client : {&net.first, &net.second}) {
        CHECK(client->networked() == 2);
        const Entity ca = client->mirror(net.server, a);
        const Entity cb = client->mirror(net.server, b);
        REQUIRE(client->w().alive(ca));
        REQUIRE(client->w().alive(cb));
        CHECK(client->w().get<Position>(ca).x == Approx(10));
        CHECK(client->w().get<Position>(cb).y == Approx(40));
        CHECK(client->w().get<Shield>(ca).value == 5);
        CHECK(client->w().get<Target>(ca).entity == cb);
        CHECK(client->w().get<Health>(ca).hp == 3);
        CHECK(client->w().get<Sprite>(ca).layer == 30);
        CHECK(client->assets.pathOf(client->w().get<Sprite>(ca).texture) == "assets/ship.png");
        CHECK(client->w().get<NetPrefab>(ca).prefabKey == prefabKey("prefabs/ship.yaml"));
    }
}

TEST_CASE("server changes and destroys reach every client", "[net][replication]") {
    Network net;
    const Entity a = net.server.spawn("prefabs/ship.yaml", {10, 20});
    const Entity b = net.server.spawn("prefabs/ship.yaml", {50, 60});
    net.run(3);
    const Entity ca = net.first.mirror(net.server, a);
    const Entity cb = net.second.mirror(net.server, b);

    net.server.w().get<Position>(a).x = 99;
    net.server.w().destroy(b);
    net.run(3);
    CHECK(net.first.w().get<Position>(ca).x == Approx(99));
    CHECK_FALSE(net.second.w().alive(cb));
    CHECK(net.first.networked() == 1);
    CHECK(net.second.networked() == 1);
}

TEST_CASE("clients follow the receiver rules of RFC 9.2", "[net][replication]") {
    Hub hub;
    Peer client{"rtype-repl-rules", hub, Endpoint::ipv4(10, 0, 0, 9, 3), false};
    const uint32_t ship = prefabKey("prefabs/ship.yaml");

    client.receiveSnapshot(SnapshotWriter(3, 10, 1).spawn(1, ship, Vec2{1, 1}).bytes);
    client.receiveSnapshot(SnapshotWriter(0, 10, 1).update(2, {5, 5}).bytes);
    client.replication->applySnapshots(client.w(), client.time);
    CHECK(client.networked() == 0);

    client.receiveSnapshot(SnapshotWriter(0, 10, 1).spawn(1, ship, Vec2{1, 1}).bytes);
    client.receiveSnapshot(SnapshotWriter(0, 12, 1).update(1, {7, 7}).bytes);
    client.receiveSnapshot(SnapshotWriter(0, 11, 1).update(1, {3, 3}).bytes);
    client.receiveSnapshot(SnapshotWriter(0, 12, 1).spawn(1, ship, Vec2{9, 9}).bytes);
    client.replication->applySnapshots(client.w(), client.time);
    const Entity e = client.replication->ids().find(1);
    REQUIRE(client.w().alive(e));
    CHECK(client.w().get<Position>(e).x == Approx(7));
    CHECK(client.w().get<Health>(e).hp == 3);

    client.receiveSnapshot(SnapshotWriter(0, 13, 2).destroy(1).destroy(4).bytes);
    client.receiveSnapshot(SnapshotWriter(0, 14, 2).spawn(1, ship).spawn(4, ship).bytes);
    client.receiveSnapshot(SnapshotWriter(0, 14, 1).spawn(9, 0xDEADBEEF, Vec2{2, 2}).bytes);
    client.receiveSnapshot(std::vector<std::byte>{std::byte{0}, std::byte{0}, std::byte{1}});
    client.replication->applySnapshots(client.w(), client.time);
    CHECK_FALSE(client.w().alive(e));
    CHECK(client.replication->ids().find(4) == Entity::null());
    CHECK(client.networked() == 1);
    CHECK(client.w().get<Position>(client.replication->ids().find(9)).x == Approx(2));
}

TEST_CASE("inputs reach the owner's RemoteInput, one per tick, and come back as lastInputProcessed",
          "[net][replication]") {
    Network net;
    const Entity ship = net.server.spawn("prefabs/ship.yaml", {});
    net.server.w().emplace<NetOwner>(ship, NetOwner{FirstPlayer});
    const std::array<ActionId, 2> order{Fire, Up};
    net.firstInput = ActionState::fromMask(0b01, 0, order);
    net.run(4);

    const RemoteInput& input = net.server.w().get<RemoteInput>(ship);
    CHECK(input.state.held(Fire));
    CHECK_FALSE(input.state.held(Up));
    CHECK(input.lastMask == 0b01);
    CHECK(input.lastTick > 0);
    CHECK(net.server.replication->lastInputProcessed(FirstPlayer) == input.lastTick);
    CHECK(net.first.replication->lastInputProcessed() > 0);
    CHECK(net.first.w().get<NetOwner>(net.first.mirror(net.server, ship)).owner == FirstPlayer);
}

TEST_CASE("the server applies redundant inputs oldest first and ignores bits beyond the actions",
          "[net][replication]") {
    Hub hub;
    Peer server{"rtype-repl-inputs", hub, Endpoint::ipv4(10, 0, 0, 9, 4), true};
    const Endpoint client = Endpoint::ipv4(10, 0, 0, 9, 5);
    const ConnectionId from = server.session->openConnection(client, SessionKey, FirstPlayer);
    const Entity ship = server.w().create();
    server.w().emplace<NetOwner>(ship, NetOwner{FirstPlayer});

    std::vector<std::byte> bytes;
    ByteWriter out(bytes, 64);
    out.write(uint16_t{0});
    out.write(uint32_t{10});
    out.write(uint8_t{4});
    for (uint64_t mask : {uint64_t{0b11}, uint64_t{0b01}, uint64_t{0b10}, uint64_t{0xF0} | 0b01})
        out.write(mask);
    server.replication->receive(InboundMessage{from, InputMessage, std::nullopt, bytes});

    server.replication->applyInputs(server.w());
    CHECK(server.w().get<RemoteInput>(ship).lastTick == 7);
    CHECK(server.w().get<RemoteInput>(ship).lastMask == 0b01);
    server.replication->applyInputs(server.w());
    CHECK(server.w().get<RemoteInput>(ship).lastTick == 8);
    CHECK(server.w().get<RemoteInput>(ship).state.pressed(Up));
    server.replication->applyInputs(server.w());
    server.replication->applyInputs(server.w());
    server.replication->applyInputs(server.w());
    CHECK(server.w().get<RemoteInput>(ship).lastTick == 10);
    CHECK_FALSE(server.w().get<RemoteInput>(ship).state.pressed(Fire));
    CHECK(server.w().get<RemoteInput>(ship).state.held(Fire));
}

TEST_CASE("interpolation draws other entities between their two latest states", "[net][replication]") {
    Hub hub;
    Peer client{"rtype-repl-lerp", hub, Endpoint::ipv4(10, 0, 0, 9, 6), false};
    const uint32_t ship = prefabKey("prefabs/ship.yaml");
    client.receiveSnapshot(SnapshotWriter(0, 10, 1).spawn(1, ship, Vec2{0, 0}).bytes);
    client.replication->applySnapshots(client.w(), client.time);
    client.receiveSnapshot(SnapshotWriter(0, 16, 1).update(1, {60, 0}).bytes);
    client.replication->applySnapshots(client.w(), client.time);
    const Entity e = client.replication->ids().find(1);

    client.replication->interpolate(client.w(), client.time);
    CHECK(client.w().get<Position>(e).x == Approx(0).margin(1e-3));
    client.time.total += 0.05;
    client.replication->interpolate(client.w(), client.time);
    CHECK(client.w().get<Position>(e).x == Approx(30).margin(1e-3));
    client.time.total += 1.0;
    client.replication->interpolate(client.w(), client.time);
    CHECK(client.w().get<Position>(e).x == Approx(60).margin(1e-3));
}
