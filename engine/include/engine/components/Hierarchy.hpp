/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Hierarchy
*/

#pragma once

#include "engine/core/math/Vec2.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <vector>

namespace engine {

struct Parent {
    Entity value;
    Vec2 offset;
};

struct Children {
    std::vector<Entity> list;
};

}

ENGINE_COMPONENT(engine::Parent)
ENGINE_COMPONENT(engine::Children)
