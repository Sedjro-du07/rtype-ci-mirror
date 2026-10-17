/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Parallax
*/

#pragma once

#include "engine/reflection/TypeKey.hpp"

namespace engine {

struct Parallax {
    float factor = 1;
    float wrapWidth = 0;
};

}

ENGINE_COMPONENT(engine::Parallax)
