/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Identity
*/

#include "engine/components/Identity.hpp"

#include "engine/core/Hash.hpp"
#include "os/Os.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>

namespace engine {

Uuid Uuid::generate() {
    std::array<std::byte, 16> bytes{};
    os::secureRandom(bytes);
    Uuid uuid;
    std::memcpy(&uuid.hi, bytes.data(), sizeof(uuid.hi));
    std::memcpy(&uuid.lo, bytes.data() + sizeof(uuid.hi), sizeof(uuid.lo));
    return uuid;
}

bool Tags::has(std::string_view tag) const {
    return std::ranges::find(this->ids, fnv1a64(tag)) != this->ids.end();
}

}

size_t std::hash<engine::Uuid>::operator()(const engine::Uuid& uuid) const noexcept {
    return static_cast<size_t>(uuid.hi ^ (uuid.lo * 0x9e3779b97f4a7c15ull));
}
