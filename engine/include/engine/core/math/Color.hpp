/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Color
*/

#pragma once

#include "engine/Export.hpp"

#include <cstdint>

namespace engine {

struct ENGINE_API Color {
    uint8_t r = 255, g = 255, b = 255, a = 255;

    static Color white();
    static Color black();
    static Color red();
    static Color green();
    static Color blue();
    static Color transparent();

    bool operator==(const Color&) const = default;
};

}
