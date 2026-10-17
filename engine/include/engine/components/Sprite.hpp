/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Sprite
*/

#pragma once

#include "engine/core/math/Color.hpp"
#include "engine/core/math/Rect.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/reflection/TypeKey.hpp"
#include "engine/render/Handles.hpp"

#include <cstdint>

namespace engine {

struct Sprite {
    TextureHandle texture;
    Rect frame;
    int16_t layer = 0;
    Color tint;
    Vec2 scale{1, 1};
    Vec2 origin;
    float rotation = 0;
    bool visible = true;
};

}

ENGINE_COMPONENT(engine::Sprite)
