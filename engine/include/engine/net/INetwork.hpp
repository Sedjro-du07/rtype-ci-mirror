/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** INetwork
*/

#pragma once

#include "engine/core/Error.hpp"
#include "engine/module/IPlugin.hpp"
#include "engine/net/NetTypes.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>

namespace engine {

class INetwork : public IPlugin {
public:
    virtual Result<void> openUdp(uint16_t port) = 0;
    virtual void closeUdp() = 0;
    virtual void sendUdp(const Endpoint& to, std::span<const std::byte> bytes) = 0;
    virtual void setReceiver(std::move_only_function<void(Datagram&&)> onDatagram) = 0;

    virtual Result<void> listenTcp(uint16_t port) = 0;
    virtual Result<uint32_t> connectTcp(const Endpoint& to) = 0;
    virtual void sendTcp(uint32_t stream, std::span<const std::byte> bytes) = 0;
    virtual void setTcpReceiver(std::move_only_function<void(uint32_t stream, std::span<const std::byte>)> fn) = 0;
    virtual void closeTcp(uint32_t stream) = 0;
};

}
