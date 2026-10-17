/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Script
*/

#pragma once

#include "engine/reflection/TypeKey.hpp"
#include "engine/render/Handles.hpp"

#include <string>

namespace engine {

struct Script {
    ScriptHandle handle;
    std::string file;
};

}

ENGINE_COMPONENT(engine::Script)
