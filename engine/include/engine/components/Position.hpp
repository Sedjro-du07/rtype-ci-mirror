/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Position
*/

#pragma once

#include "engine/reflection/TypeKey.hpp"

namespace engine {

struct Position {
    float x = 0, y = 0;
};

struct PreviousPosition {
    float x = 0, y = 0;
};

}

ENGINE_COMPONENT(engine::Position)
ENGINE_COMPONENT(engine::PreviousPosition)
