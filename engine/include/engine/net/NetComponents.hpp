/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** NetComponents
*/

#pragma once

#include "engine/core/math/Vec2.hpp"
#include "engine/input/ActionState.hpp"
#include "engine/net/NetTypes.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <cstdint>

namespace engine {

struct NetworkId {
    uint32_t value = 0;
};

struct NetPrefab {
    uint32_t prefabKey = 0;
};

struct NetOwner {
    PlayerId owner = 0;
};

struct RemoteInput {
    ActionState state;
    uint64_t lastMask = 0;
    uint32_t lastTick = 0;
};

struct NetInterpolation {
    Vec2 from, to;
    double fromTime = 0, toTime = 0;
};

struct NetPriority {
    float weight = 1.0f;
};

}

ENGINE_COMPONENT(engine::NetworkId)
ENGINE_COMPONENT(engine::NetPrefab)
ENGINE_COMPONENT(engine::NetOwner)
ENGINE_COMPONENT(engine::RemoteInput)
ENGINE_COMPONENT(engine::NetInterpolation)
ENGINE_COMPONENT(engine::NetPriority)
