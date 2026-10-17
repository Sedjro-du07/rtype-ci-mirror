/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Animation
*/

#pragma once

#include "engine/core/math/Rect.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <cstdint>
#include <vector>

namespace engine {

struct Animation {
    std::vector<Rect> frames;
    float fps = 10;
    bool loop = true;
    float time = 0;
    uint32_t current = 0;
};

}

ENGINE_COMPONENT(engine::Animation)
