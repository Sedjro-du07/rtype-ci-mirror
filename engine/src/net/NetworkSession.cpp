/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** NetworkSession
*/

#include "engine/net/NetworkSession.hpp"

#include "engine/events/EngineEvents.hpp"
#include "engine/events/EventBus.hpp"
#include "engine/reflection/ByteStream.hpp"
#include "engine/reflection/TypeRegistry.hpp"

#include <algorithm>
#include <atomic>
#include <format>
#include <mutex>
#include <utility>

namespace engine {

namespace {

constexpr uint16_t Magic = 0x5254;
constexpr uint8_t Version = 1;
constexpr uint8_t KindUdpLink = 0x01;
constexpr uint8_t KindData = 0x02;
constexpr size_t HeaderBytes = 16;
constexpr size_t UdpLinkBytes = 24;
constexpr size_t MaxUdpPayload = 1024;
constexpr uint16_t FirstGameMessage = 0x0100;
constexpr uint32_t MaxPacketsPerSecond = 300;
constexpr size_t MaxStrikes = 50;
constexpr double StrikeWindow = 10.0;

uint16_t readU16(std::span<const std::byte> bytes, size_t at) {
    return static_cast<uint16_t>((std::to_integer<uint16_t>(bytes[at]) << 8) |
                                 std::to_integer<uint16_t>(bytes[at + 1]));
}

uint32_t readU32(std::span<const std::byte> bytes, size_t at) {
    return (static_cast<uint32_t>(readU16(bytes, at)) << 16) | readU16(bytes, at + 2);
}

uint64_t readU64(std::span<const std::byte> bytes, size_t at) {
    return (static_cast<uint64_t>(readU32(bytes, at)) << 32) | readU32(bytes, at + 4);
}

bool newer(uint16_t a, uint16_t b) {
    const auto diff = static_cast<uint16_t>(a - b);
    return diff != 0 && diff < 32768;
}

}

struct NetworkSession::Inbox {
    std::mutex mutex;
    std::deque<Datagram> queue;
    uint32_t maxBytes;
    uint32_t maxQueued;
    std::atomic<uint64_t> droppedSize = 0;
    std::atomic<uint64_t> droppedInbox = 0;

    Inbox(uint32_t bytes, uint32_t queued) : maxBytes(bytes), maxQueued(queued) {}
};

NetworkSession::NetworkSession(EventBus& events, Logger& log, const NetConfig& config, const TypeRegistry& types)
    : _events(&events), _log(log.tagged("net")), _config(config), _types(&types),
      _inbox(std::make_shared<Inbox>(config.maxDatagramBytes, config.maxQueuedDatagrams)) {}

NetworkSession::~NetworkSession() {
    this->close();
}

void NetworkSession::attach(INetwork& transport) {
    this->_transport = &transport;
    transport.setReceiver([inbox = this->_inbox](Datagram&& datagram) {
        if (datagram.bytes.size() > inbox->maxBytes || datagram.bytes.size() < HeaderBytes) {
            ++inbox->droppedSize;
            return;
        }
        const std::scoped_lock lock(inbox->mutex);
        if (inbox->queue.size() >= inbox->maxQueued) {
            ++inbox->droppedInbox;
            return;
        }
        inbox->queue.push_back(std::move(datagram));
    });
}

Result<void> NetworkSession::open(bool server, uint16_t port) {
    if (!this->_transport) return fail(Errc::NetworkError, "no network plugin attached");
    if (auto ok = this->_transport->openUdp(server ? port : 0); !ok) return ok;
    this->_server = server;
    this->_open = true;
    return {};
}

void NetworkSession::close() {
    std::vector<ConnectionId> ids = this->connections();
    for (ConnectionId id : ids)
        this->closeConnection(id, this->_server ? DisconnectReason::ServerShutdown : DisconnectReason::Quit);
    if (this->_open && this->_transport) this->_transport->closeUdp();
    this->_open = false;
}

ConnectionId NetworkSession::openConnection(const Endpoint& udp, uint32_t sessionKey, PlayerId player,
                                            uint32_t stream) {
    const ConnectionId id = this->_nextConnection++;
    Connection connection;
    connection.id = id;
    connection.player = player;
    connection.udp = udp;
    connection.sessionKey = sessionKey;
    connection.stream = stream;
    connection.lastReceived = this->_now;
    connection.lastSent = this->_now;
    connection.rateStart = this->_now;
    this->_byEndpoint[udp] = id;
    this->_connections.emplace(id, std::move(connection));
    return id;
}

void NetworkSession::closeConnection(ConnectionId id, DisconnectReason reason, std::string message) {
    const auto it = this->_connections.find(id);
    if (it == this->_connections.end()) return;
    const PlayerId player = it->second.player;
    this->_byEndpoint.erase(it->second.udp);
    this->_connections.erase(it);
    this->_log.info("connection {} closed ({})", id, static_cast<int>(reason));
    if (this->_server)
        this->_events->publish(PlayerLeft{player, reason, std::move(message)});
    else
        this->_events->publish(LeftServer{reason, std::move(message)});
}

void NetworkSession::onMessage(std::move_only_function<void(const InboundMessage&)> fn) {
    this->_onMessage = std::move(fn);
}

void NetworkSession::onUdpLink(std::move_only_function<void(const Endpoint&, uint64_t)> fn) {
    this->_onUdpLink = std::move(fn);
}

std::optional<Transport> NetworkSession::transportOf(uint16_t id) const {
    if (id >= FirstGameMessage) {
        const auto messages = this->_types->messages();
        const size_t index = id - FirstGameMessage;
        if (index >= messages.size()) return std::nullopt;
        return messages[index]->transport;
    }
    if (id >= 0x0040 && id <= 0x0043) return Transport::Reliable;
    if (id == 0x0060 || id == 0x0061) return Transport::Unreliable;
    if ((id >= 0x0001 && id <= 0x0008) || (id >= 0x0020 && id <= 0x002A)) return Transport::Stream;
    return std::nullopt;
}

bool NetworkSession::admitRate(Connection& connection) {
    if (this->_now - connection.rateStart >= 1.0) {
        connection.rateStart = this->_now;
        connection.rateCount = 0;
    }
    if (++connection.rateCount <= MaxPacketsPerSecond) return true;
    ++this->_counters.droppedRate;
    return false;
}

void NetworkSession::strike(Connection& connection) {
    ++connection.stats.dropped;
    connection.strikes.push_back(this->_now);
    while (!connection.strikes.empty() && this->_now - connection.strikes.front() > StrikeWindow)
        connection.strikes.pop_front();
    if (connection.strikes.size() <= MaxStrikes) return;
    ++this->_counters.closedProtocol;
    this->_log.warn("connection {} sent too many invalid packets", connection.id);
    this->closeConnection(connection.id, DisconnectReason::ProtocolError, "too many invalid packets");
}

void NetworkSession::acknowledge(Connection& connection, uint16_t ack, uint32_t ackBits) {
    for (int i = -1; i < 32; ++i) {
        if (i >= 0 && !(ackBits & (1u << i))) continue;
        const auto sequence = static_cast<uint16_t>(ack - 1 - i);
        Sent& sent = connection.history[sequence % connection.history.size()];
        if (sent.time < 0 || sent.sequence != sequence || sent.acked) continue;
        sent.acked = true;
        const double sample = (this->_now - sent.time) * 1000.0;
        connection.stats.rttMs = connection.rttKnown ? connection.stats.rttMs * 0.9 + sample * 0.1 : sample;
        connection.rttKnown = true;
    }
}

void NetworkSession::receive(Datagram& datagram) {
    const std::span<const std::byte> bytes = datagram.bytes;
    if (bytes.size() < HeaderBytes || bytes.size() > this->_config.maxDatagramBytes) {
        ++this->_counters.droppedSize;
        return;
    }
    if (readU16(bytes, 0) != Magic || std::to_integer<uint8_t>(bytes[2]) != Version) {
        ++this->_counters.droppedMagic;
        return;
    }
    const auto kind = std::to_integer<uint8_t>(bytes[3]);
    if (kind == KindUdpLink) {
        if (bytes.size() != UdpLinkBytes || !this->_server) {
            ++this->_counters.droppedKind;
            return;
        }
        if (this->_onUdpLink) this->_onUdpLink(datagram.from, readU64(bytes, HeaderBytes));
        return;
    }
    if (kind != KindData) {
        ++this->_counters.droppedKind;
        return;
    }
    const auto found = this->_byEndpoint.find(datagram.from);
    if (found == this->_byEndpoint.end() || this->_connections.at(found->second).sessionKey != readU32(bytes, 4)) {
        ++this->_counters.droppedSession;
        return;
    }
    Connection& connection = this->_connections.at(found->second);
    if (!this->admitRate(connection)) return;

    const uint16_t sequence = readU16(bytes, 8);
    if (connection.received) {
        const auto behind = static_cast<uint16_t>(connection.remoteSequence - sequence);
        const bool duplicate =
            sequence == connection.remoteSequence || (!newer(sequence, connection.remoteSequence) &&
                                                      (behind > 32 || (connection.remoteBits & (1u << (behind - 1)))));
        if (duplicate) {
            ++this->_counters.droppedSequence;
            this->strike(connection);
            return;
        }
    }

    std::vector<InboundMessage> messages;
    size_t at = HeaderBytes;
    bool valid = true;
    while (at < bytes.size()) {
        if (bytes.size() - at < 4) {
            valid = false;
            break;
        }
        InboundMessage message;
        message.from = connection.id;
        message.id = readU16(bytes, at);
        at += 2;
        const auto transport = this->transportOf(message.id);
        if (!transport || *transport == Transport::Stream) {
            valid = false;
            break;
        }
        if (*transport == Transport::Reliable) {
            if (bytes.size() - at < 4) {
                valid = false;
                break;
            }
            message.messageSeq = readU16(bytes, at);
            at += 2;
        }
        const uint16_t length = readU16(bytes, at);
        at += 2;
        if (bytes.size() - at < length) {
            valid = false;
            break;
        }
        message.payload = bytes.subspan(at, length);
        at += length;
        messages.push_back(message);
    }
    if (!valid) {
        ++this->_counters.droppedMessage;
        this->strike(connection);
        return;
    }

    if (!connection.received || newer(sequence, connection.remoteSequence)) {
        const auto shift = static_cast<uint16_t>(sequence - connection.remoteSequence);
        connection.remoteBits = connection.received ? (shift >= 32 ? 0u : connection.remoteBits << shift) : 0u;
        if (connection.received && shift <= 32) connection.remoteBits |= 1u << (shift - 1);
        connection.remoteSequence = sequence;
        connection.received = true;
    } else {
        const auto behind = static_cast<uint16_t>(connection.remoteSequence - sequence);
        connection.remoteBits |= 1u << (behind - 1);
    }
    this->acknowledge(connection, readU16(bytes, 10), readU32(bytes, 12));
    connection.lastReceived = this->_now;
    ++connection.stats.packetsIn;
    connection.stats.bytesIn += bytes.size();
    const ConnectionId id = connection.id;
    for (const InboundMessage& message : messages) {
        if (!this->_connections.contains(id)) break;
        if (this->_onMessage) this->_onMessage(message);
    }
}

void NetworkSession::poll(double now) {
    this->_now = now;
    std::deque<Datagram> pending;
    {
        const std::scoped_lock lock(this->_inbox->mutex);
        pending.swap(this->_inbox->queue);
    }
    this->_counters.droppedSize += this->_inbox->droppedSize.exchange(0);
    this->_counters.droppedInbox += this->_inbox->droppedInbox.exchange(0);
    for (Datagram& datagram : pending)
        this->receive(datagram);

    std::vector<ConnectionId> silent;
    for (const auto& [id, connection] : this->_connections)
        if (now - connection.lastReceived > this->_config.timeoutSeconds) silent.push_back(id);
    for (ConnectionId id : silent) {
        this->_log.warn("connection {} timed out", id);
        this->closeConnection(id, DisconnectReason::Timeout);
    }
}

void NetworkSession::sendRaw(ConnectionId to, Transport transport, uint16_t messageId,
                             std::span<const std::byte> payload) {
    const auto it = this->_connections.find(to);
    if (it == this->_connections.end() || !this->_transport) return;
    Connection& connection = it->second;
    if (transport == Transport::Stream) {
        if (payload.size() + 2 > this->_config.maxStreamMessageBytes) {
            this->_log.error("message 0x{:04x} is larger than the stream limit", messageId);
            return;
        }
        std::vector<std::byte> frame;
        ByteWriter out(frame, payload.size() + 6);
        out.write(static_cast<uint32_t>(payload.size() + 2));
        out.write(messageId);
        frame.insert(frame.end(), payload.begin(), payload.end());
        this->_transport->sendTcp(connection.stream, frame);
        connection.stats.bytesOut += frame.size();
        return;
    }
    if (payload.size() > MaxUdpPayload) {
        this->_log.error("message 0x{:04x} has {} bytes, more than {} on UDP: declare it stream()", messageId,
                         payload.size(), MaxUdpPayload);
        return;
    }
    Outgoing message;
    message.id = messageId;
    if (transport == Transport::Reliable) message.messageSeq = connection.nextMessageSeq++;
    message.payload.assign(payload.begin(), payload.end());
    connection.outgoing.push_back(std::move(message));
}

void NetworkSession::sendPacket(Connection& connection, std::span<const Outgoing> messages) {
    std::vector<std::byte> packet;
    ByteWriter out(packet, this->_config.maxDatagramBytes);
    const uint16_t sequence = connection.nextSequence++;
    out.write(Magic);
    out.write(Version);
    out.write(KindData);
    out.write(connection.sessionKey);
    out.write(sequence);
    out.write(connection.received ? connection.remoteSequence : static_cast<uint16_t>(0));
    out.write(connection.remoteBits);
    for (const Outgoing& message : messages) {
        out.write(message.id);
        if (message.messageSeq) out.write(*message.messageSeq);
        out.write(static_cast<uint16_t>(message.payload.size()));
        packet.insert(packet.end(), message.payload.begin(), message.payload.end());
    }
    connection.history[sequence % connection.history.size()] = Sent{sequence, this->_now, false};
    this->_transport->sendUdp(connection.udp, packet);
    connection.lastSent = this->_now;
    ++connection.stats.packetsOut;
    connection.stats.bytesOut += packet.size();
}

void NetworkSession::flush(double now) {
    this->_now = now;
    if (!this->_transport) return;
    const size_t room = this->_config.maxDatagramBytes - HeaderBytes;
    for (auto& [id, connection] : this->_connections) {
        std::vector<Outgoing> batch;
        size_t used = 0;
        for (Outgoing& message : connection.outgoing) {
            const size_t size = message.payload.size() + (message.messageSeq ? 6 : 4);
            if (used + size > room) {
                this->sendPacket(connection, batch);
                batch.clear();
                used = 0;
            }
            used += size;
            batch.push_back(std::move(message));
        }
        connection.outgoing.clear();
        if (!batch.empty() || now - connection.lastSent >= this->_config.heartbeatSeconds)
            this->sendPacket(connection, batch);
    }
}

bool NetworkSession::isServer() const {
    return this->_server;
}

bool NetworkSession::connected() const {
    return this->_open && !this->_connections.empty();
}

std::vector<ConnectionId> NetworkSession::connections() const {
    std::vector<ConnectionId> ids;
    for (const auto& [id, connection] : this->_connections)
        ids.push_back(id);
    std::ranges::sort(ids);
    return ids;
}

ConnectionStats NetworkSession::stats(ConnectionId id) const {
    const auto it = this->_connections.find(id);
    if (it == this->_connections.end()) return {};
    ConnectionStats stats = it->second.stats;
    size_t due = 0, lost = 0;
    for (const Sent& sent : it->second.history)
        if (sent.time >= 0 && this->_now - sent.time >= 1.0) {
            ++due;
            if (!sent.acked) ++lost;
        }
    stats.lossPercent = due ? 100.0 * static_cast<double>(lost) / static_cast<double>(due) : 0.0;
    return stats;
}

const NetCounters& NetworkSession::counters() const {
    return this->_counters;
}

}
