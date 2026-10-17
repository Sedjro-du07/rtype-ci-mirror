/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Hash
*/

#pragma once

#include "engine/core/Hash.hpp"

#include <cstdint>
#include <string_view>

namespace engine {

constexpr uint64_t fnv1a64(std::string_view text) noexcept {
    uint64_t hash = 0xcbf29ce484222325ull;
    for (char c : text) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 0x100000001b3ull;
    }
    return hash;
}

}
