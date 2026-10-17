/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** AsioNetworkTest
*/

#include "AsioNetwork.hpp"

#include "engine/EngineConfig.hpp"
#include "engine/events/EventBus.hpp"
#include "engine/net/NetworkSession.hpp"
#include "engine/reflection/TypeRegistry.hpp"
#include "engine/services/CVars.hpp"

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace asiotest {

struct Ping {
    uint32_t n = 0;
};

}

ENGINE_MESSAGE(asiotest::Ping)

using namespace engine;

namespace {

Logger quietLog() {
    LogConfig settings;
    settings.file.clear();
    settings.color = false;
    settings.level = LogLevel::Error;
    return Logger(settings);
}

struct Plugin {
    Logger log = quietLog();
    CVars cvars;
    EngineConfig config;
    EventBus events;
    AsioNetwork network;

    Plugin() { REQUIRE(this->network.init(PluginContext{this->log, this->cvars, this->config, this->events})); }
};

bool waitFor(const std::function<bool()>& done) {
    const auto limit = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!done()) {
        if (std::chrono::steady_clock::now() > limit) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return true;
}

std::vector<std::byte> bytesOf(std::string_view text) {
    std::vector<std::byte> out;
    for (char c : text)
        out.push_back(static_cast<std::byte>(c));
    return out;
}

std::string textOf(std::span<const std::byte> bytes) {
    std::string out;
    for (std::byte b : bytes)
        out.push_back(static_cast<char>(b));
    return out;
}

Endpoint local(uint16_t port) {
    return Endpoint::ipv4(127, 0, 0, 1, port);
}

}

TEST_CASE("datagrams cross the loopback both ways", "[net][asio]") {
    Plugin server;
    Plugin client;
    std::mutex mutex;
    std::vector<Datagram> received;
    server.network.setReceiver([&](Datagram&& d) {
        const std::scoped_lock lock(mutex);
        received.push_back(std::move(d));
    });
    REQUIRE(server.network.openUdp(0));
    REQUIRE(client.network.openUdp(0));
    CHECK(server.network.udpPort() != 0);

    client.network.sendUdp(local(server.network.udpPort()), bytesOf("ping"));
    REQUIRE(waitFor([&] {
        const std::scoped_lock lock(mutex);
        return !received.empty();
    }));
    const std::scoped_lock lock(mutex);
    CHECK(textOf(received[0].bytes) == "ping");
    CHECK(received[0].from.port == client.network.udpPort());
    CHECK_FALSE(received[0].from.v6);
    CHECK(received[0].from.address[0] == 127);
}

TEST_CASE("stream bytes arrive in order and closing is reported", "[net][asio]") {
    Plugin server;
    Plugin client;
    std::mutex mutex;
    std::string atServer, atClient;
    std::atomic<uint32_t> serverStream = 0;
    std::atomic<bool> closed = false;
    server.network.setTcpReceiver([&](uint32_t stream, std::span<const std::byte> bytes) {
        if (bytes.empty()) {
            closed = true;
            return;
        }
        serverStream = stream;
        const std::scoped_lock lock(mutex);
        atServer += textOf(bytes);
    });
    client.network.setTcpReceiver([&](uint32_t, std::span<const std::byte> bytes) {
        const std::scoped_lock lock(mutex);
        atClient += textOf(bytes);
    });
    REQUIRE(server.network.listenTcp(0));
    const auto stream = client.network.connectTcp(local(server.network.tcpPort()));
    REQUIRE(stream);
    client.network.sendTcp(*stream, bytesOf("hello "));
    client.network.sendTcp(*stream, bytesOf("server"));
    REQUIRE(waitFor([&] {
        const std::scoped_lock lock(mutex);
        return atServer == "hello server";
    }));
    server.network.sendTcp(serverStream, bytesOf("hi"));
    REQUIRE(waitFor([&] {
        const std::scoped_lock lock(mutex);
        return atClient == "hi";
    }));
    client.network.closeTcp(*stream);
    CHECK(waitFor([&] { return closed.load(); }));
}

TEST_CASE("a refused connection is reported as closed", "[net][asio]") {
    Plugin client;
    std::atomic<bool> closed = false;
    client.network.setTcpReceiver([&](uint32_t, std::span<const std::byte> bytes) {
        if (bytes.empty()) closed = true;
    });
    Plugin probe;
    REQUIRE(probe.network.listenTcp(0));
    const uint16_t port = probe.network.tcpPort();
    probe.network.shutdown();
    REQUIRE(client.network.connectTcp(local(port)));
    CHECK(waitFor([&] { return closed.load(); }));
}

TEST_CASE("shutting down during traffic does not crash", "[net][asio]") {
    Plugin server;
    Plugin client;
    REQUIRE(server.network.openUdp(0));
    REQUIRE(server.network.listenTcp(0));
    REQUIRE(client.network.openUdp(0));
    const auto stream = client.network.connectTcp(local(server.network.tcpPort()));
    REQUIRE(stream);
    std::atomic<bool> stop = false;
    std::thread sender([&] {
        while (!stop) {
            client.network.sendUdp(local(server.network.udpPort()), bytesOf("spam"));
            client.network.sendTcp(*stream, bytesOf("spam"));
        }
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    server.network.shutdown();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    stop = true;
    sender.join();
    client.network.shutdown();
    SUCCEED();
}

TEST_CASE("an uninitialised plugin refuses to open ports", "[net][asio]") {
    AsioNetwork network;
    CHECK_FALSE(network.openUdp(0));
    CHECK_FALSE(network.listenTcp(0));
    CHECK_FALSE(network.connectTcp(local(1)));
}

TEST_CASE("two sessions exchange messages over real UDP", "[net][asio]") {
    TypeRegistry types;
    types.add<asiotest::Ping>().field("n", &asiotest::Ping::n).unreliable();
    Plugin serverSide;
    Plugin clientSide;
    NetConfig config;
    NetworkSession server(serverSide.events, serverSide.log, config, types);
    NetworkSession client(clientSide.events, clientSide.log, config, types);
    server.attach(serverSide.network);
    client.attach(clientSide.network);
    REQUIRE(server.open(true, 0));
    REQUIRE(client.open(false, 0));
    const ConnectionId toClient = server.openConnection(local(clientSide.network.udpPort()), 0x1234, 1);
    const ConnectionId toServer = client.openConnection(local(serverSide.network.udpPort()), 0x1234, 0);

    std::vector<std::byte> payload;
    server.onMessage([&](const InboundMessage& m) {
        if (m.from == toClient && m.id == 0x0100) payload.assign(m.payload.begin(), m.payload.end());
    });
    const std::vector<std::byte> four{std::byte{0}, std::byte{0}, std::byte{0}, std::byte{42}};
    client.sendRaw(toServer, Transport::Unreliable, 0x0100, four);
    client.flush(0.0);
    REQUIRE(waitFor([&] {
        server.poll(0.01);
        return !payload.empty();
    }));
    CHECK(payload == four);
    CHECK(server.stats(toClient).packetsIn == 1);
    CHECK(server.counters().droppedSession == 0);
}
