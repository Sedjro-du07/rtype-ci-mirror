// 2D vector of the engine's own vocabulary: no backend type leaves its plugin (ADR 0004).
// ENG-04 · docs/engine/reference/01-core.md

#pragma once

#include <cmath>

namespace engine {

struct Vec2 {
    float x = 0, y = 0;

    constexpr Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
    constexpr Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
    constexpr Vec2 operator*(float k) const { return {x * k, y * k}; }
    constexpr Vec2 operator/(float k) const { return {x / k, y / k}; }
    constexpr Vec2& operator+=(Vec2 o) {
        x += o.x;
        y += o.y;
        return *this;
    }
    constexpr Vec2& operator-=(Vec2 o) {
        x -= o.x;
        y -= o.y;
        return *this;
    }
    constexpr Vec2& operator*=(float k) {
        x *= k;
        y *= k;
        return *this;
    }
    constexpr bool operator==(const Vec2&) const = default;

    constexpr float dot(Vec2 o) const { return x * o.x + y * o.y; }
    float length() const { return std::sqrt(dot(*this)); }

    // Same direction, length 1. {0, 0} stays {0, 0}.
    Vec2 normalized() const {
        const float len = length();
        return len > 0.f ? Vec2{x / len, y / len} : Vec2{};
    }

    // The point at t between a (t = 0) and b (t = 1).
    static constexpr Vec2 lerp(Vec2 a, Vec2 b, float t) { return a + (b - a) * t; }
};

} // namespace engine
