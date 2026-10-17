/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Lifetime
*/

#pragma once

#include "engine/reflection/TypeKey.hpp"

namespace engine {

struct Lifetime {
    float seconds = 1;
};

}

ENGINE_COMPONENT(engine::Lifetime)
