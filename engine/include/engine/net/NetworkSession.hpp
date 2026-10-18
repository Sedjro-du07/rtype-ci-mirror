/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** NetworkSession
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Error.hpp"
#include "engine/net/INetwork.hpp"
#include "engine/net/NetTypes.hpp"
#include "engine/reflection/TypeInfo.hpp"
#include "engine/services/Logger.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

class EventBus;
class TypeRegistry;

struct NetCounters {
    uint64_t droppedSize = 0;
    uint64_t droppedMagic = 0;
    uint64_t droppedKind = 0;
    uint64_t droppedSession = 0;
    uint64_t droppedSequence = 0;
    uint64_t droppedMessage = 0;
    uint64_t droppedRate = 0;
    uint64_t droppedInbox = 0;
    uint64_t closedProtocol = 0;
};

struct InboundMessage {
    ConnectionId from = 0;
    uint16_t id = 0;
    std::optional<uint16_t> messageSeq;
    std::span<const std::byte> payload;
};

class ENGINE_API NetworkSession {
private:
    struct Inbox;

    struct Outgoing {
        uint16_t id = 0;
        std::optional<uint16_t> messageSeq;
        std::vector<std::byte> payload;
    };

    struct Sent {
        uint16_t sequence = 0;
        double time = -1;
        bool acked = false;
    };

    struct Connection {
        ConnectionId id = 0;
        PlayerId player = 0;
        Endpoint udp;
        uint32_t sessionKey = 0;
        uint32_t stream = 0;
        uint16_t nextSequence = 1;
        uint16_t remoteSequence = 0;
        uint32_t remoteBits = 0;
        bool received = false;
        uint16_t nextMessageSeq = 0;
        double lastReceived = 0;
        double lastSent = 0;
        std::vector<Outgoing> outgoing;
        std::array<Sent, 64> history{};
        ConnectionStats stats;
        bool rttKnown = false;
        std::deque<double> strikes;
        double rateStart = 0;
        uint32_t rateCount = 0;
    };

    EventBus* _events;
    Logger _log;
    NetConfig _config;
    const TypeRegistry* _types;
    INetwork* _transport = nullptr;
    std::shared_ptr<Inbox> _inbox;
    std::unordered_map<ConnectionId, Connection> _connections;
    std::unordered_map<Endpoint, ConnectionId> _byEndpoint;
    ConnectionId _nextConnection = 1;
    bool _server = false;
    bool _open = false;
    double _now = 0;
    NetCounters _counters;
    std::move_only_function<void(const InboundMessage&)> _onMessage;
    std::move_only_function<void(const Endpoint&, uint64_t)> _onUdpLink;

    std::optional<Transport> transportOf(uint16_t id) const;
    void receive(Datagram& datagram);
    bool admitRate(Connection& connection);
    void strike(Connection& connection);
    void acknowledge(Connection& connection, uint16_t ack, uint32_t ackBits);
    void sendPacket(Connection& connection, std::span<const Outgoing> messages);

public:
    NetworkSession(EventBus& events, Logger& log, const NetConfig& config, const TypeRegistry& types);
    NetworkSession(const NetworkSession&) = delete;
    NetworkSession& operator=(const NetworkSession&) = delete;
    ~NetworkSession();

    void attach(INetwork& transport);
    Result<void> open(bool server, uint16_t port);
    void close();
    ConnectionId openConnection(const Endpoint& udp, uint32_t sessionKey, PlayerId player, uint32_t stream = 0);
    void closeConnection(ConnectionId connection, DisconnectReason reason, std::string message = {});
    void onMessage(std::move_only_function<void(const InboundMessage&)> fn);
    void onUdpLink(std::move_only_function<void(const Endpoint&, uint64_t)> fn);

    void poll(double now);
    void sendRaw(ConnectionId to, Transport transport, uint16_t messageId, std::span<const std::byte> payload);
    void flush(double now);

    bool isServer() const;
    bool connected() const;
    std::vector<ConnectionId> connections() const;
    PlayerId player(ConnectionId connection) const;
    ConnectionStats stats(ConnectionId connection) const;
    const NetCounters& counters() const;
};

}
