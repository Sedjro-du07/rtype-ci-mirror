/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** DrawCmd
*/

#pragma once

#include "engine/core/math/Color.hpp"
#include "engine/core/math/Rect.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/render/Handles.hpp"

#include <cstdint>

namespace engine {

struct DrawCmd {
    enum class Kind : uint8_t { Sprite, Text, Rect, RectOutline, Line, Circle };

    Kind kind = Kind::Sprite;
    int16_t layer = 0;
    bool screenSpace = false;
    TextureHandle texture;
    Rect source;
    Vec2 position;
    Vec2 size;
    Vec2 end;
    Vec2 scale{1, 1};
    Vec2 origin;
    float rotation = 0.f;
    Color tint;
    FontHandle font;
    uint16_t textSize = 16;
    uint32_t textOffset = 0, textLength = 0;
};

}
