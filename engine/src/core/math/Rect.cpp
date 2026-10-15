/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Rect
*/

#include "engine/core/math/Rect.hpp"

namespace engine {

bool Rect::contains(Vec2 p) const {
    return p.x >= x && p.y >= y && p.x < x + w && p.y < y + h;
}

bool Rect::intersects(const Rect& o) const {
    return x < o.x + o.w && o.x < x + w && y < o.y + o.h && o.y < y + h;
}

Vec2 Rect::center() const {
    return {x + w / 2.f, y + h / 2.f};
}

}
