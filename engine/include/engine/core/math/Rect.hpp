/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Rect
*/

#pragma once

#include "engine/core/math/Vec2.hpp"

namespace engine {

struct Rect {
    float x = 0, y = 0, w = 0, h = 0;

    constexpr bool contains(Vec2 p) const { return p.x >= x && p.y >= y && p.x < x + w && p.y < y + h; }

    constexpr bool intersects(const Rect& o) const {
        return x < o.x + o.w && o.x < x + w && y < o.y + o.h && o.y < y + h;
    }

    constexpr Vec2 center() const { return {x + w / 2.f, y + h / 2.f}; }
    constexpr bool operator==(const Rect&) const = default;
};

}
