/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Text
*/

#pragma once

#include "engine/core/math/Color.hpp"
#include "engine/reflection/TypeKey.hpp"
#include "engine/render/Handles.hpp"

#include <cstdint>
#include <string>

namespace engine {

struct Text {
    std::string value;
    FontHandle font;
    uint16_t size = 16;
    Color color;
    int16_t layer = 0;
    bool screenSpace = false;
};

}

ENGINE_COMPONENT(engine::Text)
