/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Rect
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/math/Vec2.hpp"

namespace engine {

struct ENGINE_API Rect {
    float x = 0, y = 0, w = 0, h = 0;

    bool contains(Vec2 p) const;
    bool intersects(const Rect& o) const;
    Vec2 center() const;
    bool operator==(const Rect&) const = default;
};

}
