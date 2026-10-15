// Axis-aligned rectangle: top-left corner and size.
// ENG-04 · docs/engine/reference/01-core.md

#pragma once

#include "engine/core/math/Vec2.hpp"

namespace engine {

struct Rect {
    float x = 0, y = 0, w = 0, h = 0;

    // Left and top edges included, right and bottom edges excluded: side-by-side rectangles share no point.
    constexpr bool contains(Vec2 p) const { return p.x >= x && p.y >= y && p.x < x + w && p.y < y + h; }

    // Rectangles that only touch do not intersect (a ship against a wall is not colliding).
    constexpr bool intersects(const Rect& o) const {
        return x < o.x + o.w && o.x < x + w && y < o.y + o.h && o.y < y + h;
    }

    constexpr Vec2 center() const { return {x + w / 2.f, y + h / 2.f}; }
    constexpr bool operator==(const Rect&) const = default;
};

} // namespace engine
