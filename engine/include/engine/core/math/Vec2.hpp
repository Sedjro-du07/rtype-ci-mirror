/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Vec2
*/

#pragma once

#include "engine/Export.hpp"

namespace engine {

struct ENGINE_API Vec2 {
    float x = 0, y = 0;

    Vec2 operator+(Vec2 o) const;
    Vec2 operator-(Vec2 o) const;
    Vec2 operator*(float k) const;
    Vec2 operator/(float k) const;
    Vec2& operator+=(Vec2 o);
    Vec2& operator-=(Vec2 o);
    Vec2& operator*=(float k);
    bool operator==(const Vec2&) const = default;

    float dot(Vec2 o) const;
    float length() const;
    Vec2 normalized() const;
    static Vec2 lerp(Vec2 a, Vec2 b, float t);
};

}
