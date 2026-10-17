/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Camera2D
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/render/WindowDesc.hpp"

namespace engine {

struct ENGINE_API Camera2D {
    Vec2 center{640, 360};
    float zoom = 1.f;
    float rotation = 0.f;

    Vec2 screenToWorld(Vec2 screen, const WindowDesc& window) const;
    Vec2 worldToScreen(Vec2 world, const WindowDesc& window) const;
};

}
