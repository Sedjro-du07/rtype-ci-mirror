/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** MovementSystems
*/

#include "engine/components/Hierarchy.hpp"
#include "engine/components/Lifetime.hpp"
#include "engine/components/Parallax.hpp"
#include "engine/components/Position.hpp"
#include "engine/components/Velocity.hpp"
#include "engine/ecs/Commands.hpp"
#include "engine/ecs/Registry.hpp"
#include "systems/EngineSystems.hpp"

#include <cmath>
#include <vector>

namespace engine {

namespace {

constexpr int MaxDepth = 32;

Position placeOf(Registry& world, Entity e, int depth) {
    const Position& own = world.get<Position>(e);
    const Parent* parent = world.tryGet<Parent>(e);
    if (!parent || parent->value.isNull() || !world.alive(parent->value) || depth >= MaxDepth) return own;
    if (!world.has<Position>(parent->value)) return own;
    const Position base = placeOf(world, parent->value, depth + 1);
    return {base.x + parent->offset.x, base.y + parent->offset.y};
}

}

void previousPosition(Registry& world) {
    std::vector<Entity> missing;
    for (auto [e, position] : world.view<Position>()) {
        if (auto* previous = world.tryGet<PreviousPosition>(e))
            *previous = {position.x, position.y};
        else
            missing.push_back(e);
    }
    for (Entity e : missing) {
        const Position& position = world.get<Position>(e);
        world.emplace<PreviousPosition>(e, position.x, position.y);
    }
}

void movement(Registry& world, double dt) {
    const auto step = static_cast<float>(dt);
    for (auto [e, position, velocity] : world.view<Position, Velocity>()) {
        position.x += velocity.dx * step;
        position.y += velocity.dy * step;
    }
}

void hierarchy(Registry& world) {
    std::vector<std::pair<Entity, Position>> placed;
    for (auto [e, parent, position] : world.view<Parent, Position>())
        placed.emplace_back(e, placeOf(world, e, 0));
    for (const auto& [e, position] : placed)
        world.get<Position>(e) = position;
}

void parallax(Registry& world) {
    for (auto [e, layer, position] : world.view<Parallax, Position>()) {
        if (layer.wrapWidth <= 0) continue;
        position.x = std::fmod(position.x, layer.wrapWidth);
        if (position.x > 0) position.x -= layer.wrapWidth;
    }
}

void lifetime(Registry& world, Commands& commands, double dt) {
    for (auto [e, life] : world.view<Lifetime>()) {
        const bool alive = life.seconds > 0;
        life.seconds -= static_cast<float>(dt);
        if (alive && life.seconds <= 0) commands.destroy(e);
    }
}

}
