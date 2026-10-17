/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Vec2
*/

#include "engine/core/math/Vec2.hpp"

#include <cmath>

namespace engine {

Vec2 Vec2::operator+(Vec2 o) const {
    return {x + o.x, y + o.y};
}

Vec2 Vec2::operator-(Vec2 o) const {
    return {x - o.x, y - o.y};
}

Vec2 Vec2::operator*(float k) const {
    return {x * k, y * k};
}

Vec2 Vec2::operator/(float k) const {
    return {x / k, y / k};
}

Vec2& Vec2::operator+=(Vec2 o) {
    x += o.x;
    y += o.y;
    return *this;
}

Vec2& Vec2::operator-=(Vec2 o) {
    x -= o.x;
    y -= o.y;
    return *this;
}

Vec2& Vec2::operator*=(float k) {
    x *= k;
    y *= k;
    return *this;
}

float Vec2::dot(Vec2 o) const {
    return x * o.x + y * o.y;
}

float Vec2::length() const {
    return std::sqrt(dot(*this));
}

Vec2 Vec2::normalized() const {
    const float len = length();
    return len > 0.f ? Vec2{x / len, y / len} : Vec2{};
}

Vec2 Vec2::lerp(Vec2 a, Vec2 b, float t) {
    return a + (b - a) * t;
}

}
