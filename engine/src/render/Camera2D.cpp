/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Camera2D
*/

#include "engine/render/Camera2D.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace engine {

namespace {

struct Letterbox {
    float scale;
    Vec2 offset;
};

Letterbox letterbox(const WindowDesc& window) {
    const float vw = static_cast<float>(window.virtualWidth);
    const float vh = static_cast<float>(window.virtualHeight);
    const float scale = std::min(static_cast<float>(window.width) / vw, static_cast<float>(window.height) / vh);
    return {scale, Vec2{(static_cast<float>(window.width) - vw * scale) / 2.f,
                        (static_cast<float>(window.height) - vh * scale) / 2.f}};
}

Vec2 rotated(Vec2 v, float degrees) {
    const float radians = degrees * std::numbers::pi_v<float> / 180.f;
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    return {v.x * c - v.y * s, v.x * s + v.y * c};
}

Vec2 virtualCenter(const WindowDesc& window) {
    return {static_cast<float>(window.virtualWidth) / 2.f, static_cast<float>(window.virtualHeight) / 2.f};
}

}

Vec2 Camera2D::worldToScreen(Vec2 world, const WindowDesc& window) const {
    const Letterbox box = letterbox(window);
    const Vec2 view = rotated(world - this->center, -this->rotation) * this->zoom + virtualCenter(window);
    return view * box.scale + box.offset;
}

Vec2 Camera2D::screenToWorld(Vec2 screen, const WindowDesc& window) const {
    const Letterbox box = letterbox(window);
    const Vec2 view = (screen - box.offset) / box.scale;
    return rotated((view - virtualCenter(window)) / this->zoom, this->rotation) + this->center;
}

}
