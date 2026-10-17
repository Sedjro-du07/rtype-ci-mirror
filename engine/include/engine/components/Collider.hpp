/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Collider
*/

#pragma once

#include "engine/core/math/Vec2.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <cstdint>

namespace engine {

struct Collider {
    Vec2 size;
    Vec2 offset;
    uint32_t layer = 1;
    uint32_t mask = 0xFFFFFFFFu;
    bool trigger = false;
};

}

ENGINE_COMPONENT(engine::Collider)
