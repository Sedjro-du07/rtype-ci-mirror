/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** NetTypes
*/

#include "engine/net/NetTypes.hpp"

#include "os/Os.hpp"

#include <format>
#include <string>

namespace engine {

std::string Endpoint::toString() const {
    if (!this->v6)
        return std::format("{}.{}.{}.{}:{}", this->address[0], this->address[1], this->address[2], this->address[3],
                           this->port);
    std::array<uint16_t, 8> groups{};
    for (size_t i = 0; i < 8; ++i)
        groups[i] = static_cast<uint16_t>((this->address[i * 2] << 8) | this->address[i * 2 + 1]);
    size_t bestStart = 8, bestLength = 0;
    for (size_t i = 0; i < 8;) {
        size_t j = i;
        while (j < 8 && groups[j] == 0)
            ++j;
        if (j - i > bestLength && j - i >= 2) {
            bestStart = i;
            bestLength = j - i;
        }
        i = j == i ? i + 1 : j;
    }
    std::string text = "[";
    for (size_t i = 0; i < 8; ++i) {
        if (i == bestStart) {
            text += "::";
            i += bestLength - 1;
            continue;
        }
        if (i != 0 && i != bestStart + bestLength) text += ':';
        text += std::format("{:x}", groups[i]);
    }
    return text + std::format("]:{}", this->port);
}

Result<Endpoint> Endpoint::resolve(std::string_view host, uint16_t port) {
    auto found = os::resolveHost(host);
    if (!found) return unexpected(found.error());
    const os::HostAddress& first = found->front();
    Endpoint endpoint;
    endpoint.address = first.bytes;
    endpoint.v6 = first.v6;
    endpoint.port = port;
    return endpoint;
}

Endpoint Endpoint::ipv4(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port) {
    Endpoint endpoint;
    endpoint.address[0] = a;
    endpoint.address[1] = b;
    endpoint.address[2] = c;
    endpoint.address[3] = d;
    endpoint.port = port;
    return endpoint;
}

}

size_t std::hash<engine::Endpoint>::operator()(const engine::Endpoint& endpoint) const noexcept {
    size_t hash = endpoint.v6 ? 1469598103934665603ull : 7809847782465536322ull;
    for (uint8_t byte : endpoint.address)
        hash = (hash ^ byte) * 1099511628211ull;
    return (hash ^ endpoint.port) * 1099511628211ull;
}
