/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** AsioNetwork
*/

#include "AsioNetwork.hpp"

#include "engine/module/Export.hpp"

#include <algorithm>
#include <cstring>
#include <future>
#include <string>
#include <system_error>
#include <utility>

namespace engine {

namespace {

asio::ip::address addressOf(const Endpoint& endpoint) {
    if (!endpoint.v6) {
        asio::ip::address_v4::bytes_type bytes{};
        std::copy_n(endpoint.address.begin(), 4, bytes.begin());
        return asio::ip::make_address_v6(asio::ip::v4_mapped, asio::ip::address_v4(bytes));
    }
    asio::ip::address_v6::bytes_type bytes{};
    std::copy_n(endpoint.address.begin(), 16, bytes.begin());
    return asio::ip::address_v6(bytes);
}

Endpoint endpointOf(const asio::ip::address& address, uint16_t port) {
    Endpoint endpoint;
    endpoint.port = port;
    if (address.is_v6() && address.to_v6().is_v4_mapped()) {
        const auto bytes = asio::ip::make_address_v4(asio::ip::v4_mapped, address.to_v6()).to_bytes();
        std::copy(bytes.begin(), bytes.end(), endpoint.address.begin());
        return endpoint;
    }
    if (address.is_v4()) {
        const auto bytes = address.to_v4().to_bytes();
        std::copy(bytes.begin(), bytes.end(), endpoint.address.begin());
        return endpoint;
    }
    const auto bytes = address.to_v6().to_bytes();
    std::copy(bytes.begin(), bytes.end(), endpoint.address.begin());
    endpoint.v6 = true;
    return endpoint;
}

asio::ip::tcp::endpoint tcpEndpoint(const Endpoint& endpoint) {
    if (endpoint.v6) return {addressOf(endpoint), endpoint.port};
    asio::ip::address_v4::bytes_type bytes{};
    std::copy_n(endpoint.address.begin(), 4, bytes.begin());
    return {asio::ip::address_v4(bytes), endpoint.port};
}

template <class Fn>
auto onIo(asio::io_context& io, Fn fn) {
    using Out = decltype(fn());
    auto promise = std::make_shared<std::promise<Out>>();
    auto result = promise->get_future();
    asio::post(io, [promise, fn = std::move(fn)]() mutable { promise->set_value(fn()); });
    return result.get();
}

}

AsioNetwork::Stream::Stream(asio::io_context& io) : socket(io) {}

AsioNetwork::AsioNetwork() = default;

AsioNetwork::~AsioNetwork() {
    this->shutdown();
}

PluginKind AsioNetwork::kind() const {
    return PluginKind::Network;
}

std::string_view AsioNetwork::name() const {
    return "asio";
}

uint32_t AsioNetwork::apiVersion() const {
    return ENGINE_PLUGIN_API_VERSION;
}

Result<void> AsioNetwork::init(const PluginContext&) {
    if (this->_thread.joinable()) return {};
    this->_io.restart();
    this->_work.emplace(asio::make_work_guard(this->_io));
    this->_thread = std::thread([this] { this->_io.run(); });
    return {};
}

void AsioNetwork::shutdown() noexcept {
    if (!this->_thread.joinable()) return;
    asio::post(this->_io, [this] {
        std::error_code ignored;
        if (this->_udp) this->_udp->close(ignored);
        if (this->_acceptor) this->_acceptor->close(ignored);
        for (auto& [id, stream] : this->_streams)
            stream->socket.close(ignored);
        this->_streams.clear();
    });
    this->_work.reset();
    this->_thread.join();
    this->_udp.reset();
    this->_acceptor.reset();
}

Result<void> AsioNetwork::openUdp(uint16_t port) {
    if (!this->_thread.joinable()) return fail(Errc::NetworkError, "the asio plugin is not initialised");
    return onIo(this->_io, [this, port]() -> Result<void> {
        std::error_code error;
        asio::ip::udp::socket socket(this->_io);
        socket.open(asio::ip::udp::v6(), error);
        if (!error) socket.set_option(asio::ip::v6_only(false), error);
        if (!error) socket.bind({asio::ip::udp::v6(), port}, error);
        if (error)
            return fail(Errc::NetworkError, "cannot open UDP port " + std::to_string(port) + ": " + error.message());
        this->_udpPort = socket.local_endpoint().port();
        this->_udp.emplace(std::move(socket));
        this->receiveDatagram();
        return {};
    });
}

void AsioNetwork::closeUdp() {
    asio::post(this->_io, [this] {
        std::error_code ignored;
        if (this->_udp) this->_udp->close(ignored);
    });
}

void AsioNetwork::receiveDatagram() {
    this->_udp->async_receive_from(asio::buffer(this->_datagram), this->_from,
                                   [this](const std::error_code& error, size_t size) {
                                       if (error == asio::error::operation_aborted || !this->_udp->is_open()) return;
                                       if (!error) {
                                           Datagram datagram{endpointOf(this->_from.address(), this->_from.port()),
                                                             {this->_datagram.begin(), this->_datagram.begin() + size}};
                                           const std::scoped_lock lock(this->_receivers);
                                           if (this->_onDatagram) this->_onDatagram(std::move(datagram));
                                       }
                                       this->receiveDatagram();
                                   });
}

void AsioNetwork::sendUdp(const Endpoint& to, std::span<const std::byte> bytes) {
    auto copy = std::make_shared<std::vector<std::byte>>(bytes.begin(), bytes.end());
    asio::post(this->_io, [this, to, copy] {
        if (!this->_udp || !this->_udp->is_open()) return;
        this->_udp->async_send_to(asio::buffer(*copy), {addressOf(to), to.port},
                                  [copy](const std::error_code&, size_t) {});
    });
}

void AsioNetwork::setReceiver(std::move_only_function<void(Datagram&&)> onDatagram) {
    const std::scoped_lock lock(this->_receivers);
    this->_onDatagram = std::move(onDatagram);
}

Result<void> AsioNetwork::listenTcp(uint16_t port) {
    if (!this->_thread.joinable()) return fail(Errc::NetworkError, "the asio plugin is not initialised");
    return onIo(this->_io, [this, port]() -> Result<void> {
        std::error_code error;
        asio::ip::tcp::acceptor acceptor(this->_io);
        acceptor.open(asio::ip::tcp::v6(), error);
        if (!error) acceptor.set_option(asio::ip::v6_only(false), error);
        if (!error) acceptor.set_option(asio::socket_base::reuse_address(true), error);
        if (!error) acceptor.bind({asio::ip::tcp::v6(), port}, error);
        if (!error) acceptor.listen(asio::socket_base::max_listen_connections, error);
        if (error)
            return fail(Errc::NetworkError,
                        "cannot listen on TCP port " + std::to_string(port) + ": " + error.message());
        this->_tcpPort = acceptor.local_endpoint().port();
        this->_acceptor.emplace(std::move(acceptor));
        this->accept();
        return {};
    });
}

void AsioNetwork::accept() {
    auto stream = std::make_shared<Stream>(this->_io);
    this->_acceptor->async_accept(stream->socket, [this, stream](const std::error_code& error) {
        if (error == asio::error::operation_aborted || !this->_acceptor->is_open()) return;
        if (!error) {
            stream->connected = true;
            const uint32_t id = this->_nextStream++;
            this->_streams.emplace(id, stream);
            this->readStream(id, stream);
        }
        this->accept();
    });
}

Result<uint32_t> AsioNetwork::connectTcp(const Endpoint& to) {
    if (!this->_thread.joinable()) return fail(Errc::NetworkError, "the asio plugin is not initialised");
    const uint32_t id = this->_nextStream++;
    asio::post(this->_io, [this, id, to] {
        auto stream = std::make_shared<Stream>(this->_io);
        this->_streams.emplace(id, stream);
        stream->socket.async_connect(tcpEndpoint(to), [this, id, stream](const std::error_code& error) {
            if (error) {
                this->dropStream(id);
                return;
            }
            stream->connected = true;
            this->readStream(id, stream);
            this->writeStream(id, stream);
        });
    });
    return id;
}

void AsioNetwork::deliver(uint32_t id, std::span<const std::byte> bytes) {
    const std::scoped_lock lock(this->_receivers);
    if (this->_onStream) this->_onStream(id, bytes);
}

void AsioNetwork::readStream(uint32_t id, std::shared_ptr<Stream> stream) {
    stream->socket.async_read_some(asio::buffer(stream->buffer),
                                   [this, id, stream](const std::error_code& error, size_t size) {
                                       if (error) {
                                           if (error != asio::error::operation_aborted) this->dropStream(id);
                                           return;
                                       }
                                       this->deliver(id, std::span<const std::byte>(stream->buffer.data(), size));
                                       this->readStream(id, stream);
                                   });
}

void AsioNetwork::writeStream(uint32_t id, std::shared_ptr<Stream> stream) {
    if (stream->writing || stream->outgoing.empty() || !stream->connected) return;
    stream->writing = true;
    asio::async_write(stream->socket, asio::buffer(stream->outgoing.front()),
                      [this, id, stream](const std::error_code& error, size_t) {
                          stream->writing = false;
                          if (error) {
                              if (error != asio::error::operation_aborted) this->dropStream(id);
                              return;
                          }
                          stream->outgoing.pop_front();
                          this->writeStream(id, stream);
                      });
}

void AsioNetwork::dropStream(uint32_t id) {
    const auto it = this->_streams.find(id);
    if (it == this->_streams.end()) return;
    std::error_code ignored;
    it->second->socket.close(ignored);
    this->_streams.erase(it);
    this->deliver(id, {});
}

void AsioNetwork::sendTcp(uint32_t stream, std::span<const std::byte> bytes) {
    std::vector<std::byte> copy(bytes.begin(), bytes.end());
    asio::post(this->_io, [this, stream, copy = std::move(copy)]() mutable {
        const auto it = this->_streams.find(stream);
        if (it == this->_streams.end()) return;
        it->second->outgoing.push_back(std::move(copy));
        this->writeStream(stream, it->second);
    });
}

void AsioNetwork::setTcpReceiver(std::move_only_function<void(uint32_t, std::span<const std::byte>)> fn) {
    const std::scoped_lock lock(this->_receivers);
    this->_onStream = std::move(fn);
}

void AsioNetwork::closeTcp(uint32_t stream) {
    asio::post(this->_io, [this, stream] {
        const auto it = this->_streams.find(stream);
        if (it == this->_streams.end()) return;
        std::error_code ignored;
        it->second->socket.close(ignored);
        this->_streams.erase(it);
    });
}

uint16_t AsioNetwork::udpPort() const {
    return this->_udpPort;
}

uint16_t AsioNetwork::tcpPort() const {
    return this->_tcpPort;
}
}

ENGINE_PLUGIN(engine::AsioNetwork)
