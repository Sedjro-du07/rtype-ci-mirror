/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TestWorld
*/

#pragma once

#include "engine/ecs/Registry.hpp"
#include "engine/reflection/TypeRegistry.hpp"

#include <cstdint>
#include <string>

namespace world {

struct Position {
    float x = 0, y = 0;
};

struct Velocity {
    float x = 0, y = 0;
};

struct Health {
    int32_t hp = 100;
};

struct Frozen {};

struct Score {
    int32_t value = 0;
};

struct Unregistered {
    int32_t value = 0;
};

}

ENGINE_COMPONENT(world::Position)
ENGINE_COMPONENT(world::Velocity)
ENGINE_COMPONENT(world::Health)
ENGINE_COMPONENT(world::Frozen)
ENGINE_COMPONENT(world::Score)
ENGINE_COMPONENT(world::Unregistered)

namespace world {

inline engine::TypeRegistry makeTypes() {
    engine::TypeRegistry types;
    types.add<engine::Name>().field("value", &engine::Name::value);
    types.add<engine::Uuid>();
    types.add<engine::Tags>();
    types.add<engine::Parent>().field("value", &engine::Parent::value).field("offset", &engine::Parent::offset);
    types.add<engine::Children>().field("list", &engine::Children::list);
    types.add<Position>().field("x", &Position::x).field("y", &Position::y);
    types.add<Velocity>().field("x", &Velocity::x).field("y", &Velocity::y);
    types.add<Health>().field("hp", &Health::hp);
    types.add<Frozen>();
    types.add<Score>().field("value", &Score::value);
    return types;
}

}
