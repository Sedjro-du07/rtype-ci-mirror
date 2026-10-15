/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Entity
*/

#include "engine/ecs/Entity.hpp"

#include <cstddef>
#include <cstdint>

namespace engine {}

size_t std::hash<engine::Entity>::operator()(engine::Entity e) const noexcept {
    return static_cast<size_t>((static_cast<uint64_t>(e.generation) << 32) | e.index);
}
