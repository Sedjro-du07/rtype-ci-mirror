/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** AsioNetwork
*/

#pragma once

#include "engine/net/INetwork.hpp"

#include <asio.hpp>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

namespace engine {

class AsioNetwork final : public INetwork {
private:
    struct Stream {
        asio::ip::tcp::socket socket;
        std::array<std::byte, 4096> buffer{};
        std::deque<std::vector<std::byte>> outgoing;
        bool writing = false;
        bool connected = false;

        explicit Stream(asio::io_context& io);
    };

    asio::io_context _io;
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>> _work;
    std::thread _thread;
    std::optional<asio::ip::udp::socket> _udp;
    asio::ip::udp::endpoint _from;
    std::array<std::byte, 2048> _datagram{};
    std::optional<asio::ip::tcp::acceptor> _acceptor;
    std::unordered_map<uint32_t, std::shared_ptr<Stream>> _streams;
    std::atomic<uint32_t> _nextStream = 1;
    std::mutex _receivers;
    std::move_only_function<void(Datagram&&)> _onDatagram;
    std::move_only_function<void(uint32_t, std::span<const std::byte>)> _onStream;
    std::atomic<uint16_t> _udpPort = 0;
    std::atomic<uint16_t> _tcpPort = 0;

    void receiveDatagram();
    void accept();
    void readStream(uint32_t id, std::shared_ptr<Stream> stream);
    void writeStream(uint32_t id, std::shared_ptr<Stream> stream);
    void dropStream(uint32_t id);
    void deliver(uint32_t id, std::span<const std::byte> bytes);

public:
    AsioNetwork();
    AsioNetwork(const AsioNetwork&) = delete;
    AsioNetwork& operator=(const AsioNetwork&) = delete;
    ~AsioNetwork() override;

    PluginKind kind() const override;
    std::string_view name() const override;
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;
    void shutdown() noexcept override;

    Result<void> openUdp(uint16_t port) override;
    void closeUdp() override;
    void sendUdp(const Endpoint& to, std::span<const std::byte> bytes) override;
    void setReceiver(std::move_only_function<void(Datagram&&)> onDatagram) override;

    Result<void> listenTcp(uint16_t port) override;
    Result<uint32_t> connectTcp(const Endpoint& to) override;
    void sendTcp(uint32_t stream, std::span<const std::byte> bytes) override;
    void setTcpReceiver(std::move_only_function<void(uint32_t, std::span<const std::byte>)> fn) override;
    void closeTcp(uint32_t stream) override;

    uint16_t udpPort() const;
    uint16_t tcpPort() const;
};

}
