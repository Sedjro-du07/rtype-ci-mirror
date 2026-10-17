/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Entity
*/

#pragma once

#include "engine/Export.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>

namespace engine {

struct Entity {
    uint32_t index = UINT32_MAX;
    uint32_t generation = 0;

    static constexpr Entity null() noexcept;
    constexpr bool isNull() const noexcept;
    friend constexpr bool operator==(Entity, Entity) = default;
};

}

template <>
struct ENGINE_API std::hash<engine::Entity> {
    size_t operator()(engine::Entity e) const noexcept;
};

#include "engine/ecs/Entity.inl"
