/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Entity
*/

#pragma once

#include "engine/ecs/Entity.hpp"

#include <cstdint>

namespace engine {

constexpr Entity Entity::null() noexcept {
    return {};
}

constexpr bool Entity::isNull() const noexcept {
    return index == UINT32_MAX;
}

}
