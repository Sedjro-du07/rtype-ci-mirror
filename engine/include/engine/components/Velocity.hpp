/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Velocity
*/

#pragma once

#include "engine/reflection/TypeKey.hpp"

namespace engine {

struct Velocity {
    float dx = 0, dy = 0;
};

}

ENGINE_COMPONENT(engine::Velocity)
