/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EngineSystems
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Time.hpp"
#include "engine/core/math/Rect.hpp"

#include <functional>

namespace engine {

class Commands;
class Context;
class EventBus;
class Registry;
class RenderQueue;
class Scheduler;
struct Collider;
struct Position;

struct SystemTargets {
    Registry& world;
    Commands& commands;
    EventBus& events;
    const Time& time;
    RenderQueue* render = nullptr;
    float gridCell = 64;
    bool debugColliders = false;
};

using TargetsOf = std::function<SystemTargets(Context&)>;

ENGINE_API void previousPosition(Registry& world);
ENGINE_API void movement(Registry& world, double dt);
ENGINE_API void hierarchy(Registry& world);
ENGINE_API void parallax(Registry& world);
ENGINE_API void lifetime(Registry& world, Commands& commands, double dt);
ENGINE_API bool layersMatch(const Collider& a, const Collider& b);
ENGINE_API Rect worldBox(const Position& position, const Collider& collider);
ENGINE_API void collision(Registry& world, EventBus& events, float cellSize);
ENGINE_API void animation(Registry& world, double dt);
ENGINE_API void renderSprites(Registry& world, RenderQueue& render, double alpha);
ENGINE_API void renderText(Registry& world, RenderQueue& render);
ENGINE_API void renderDebug(Registry& world, RenderQueue& render);
ENGINE_API void registerEngineSystems(Scheduler& scheduler, TargetsOf targetsOf);

}
