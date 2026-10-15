/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Color
*/

#include "engine/core/math/Color.hpp"

namespace engine {

Color Color::white() {
    return {255, 255, 255, 255};
}

Color Color::black() {
    return {0, 0, 0, 255};
}

Color Color::red() {
    return {255, 0, 0, 255};
}

Color Color::green() {
    return {0, 255, 0, 255};
}

Color Color::blue() {
    return {0, 0, 255, 255};
}

Color Color::transparent() {
    return {0, 0, 0, 0};
}

}
