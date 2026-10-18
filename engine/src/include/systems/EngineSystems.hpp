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
#include <memory>

namespace engine {

class ActionState;
class Commands;
class Context;
class EventBus;
class Registry;
class RenderQueue;
class Replication;
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
    Replication* replication = nullptr;
    const ActionState* input = nullptr;
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
ENGINE_API void netAssignIds(SystemTargets& targets);
ENGINE_API void netSnapshot(SystemTargets& targets);
ENGINE_API void netApplySnapshot(SystemTargets& targets);
ENGINE_API void netInterpolate(SystemTargets& targets);
ENGINE_API void netSendInput(SystemTargets& targets);
ENGINE_API void netReceiveInput(SystemTargets& targets);
ENGINE_API void registerNetSystems(Scheduler& scheduler, std::shared_ptr<TargetsOf> of);
ENGINE_API void registerEngineSystems(Scheduler& scheduler, TargetsOf targetsOf);

}
