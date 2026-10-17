/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EngineEvents
*/

#pragma once

#include "engine/ecs/Entity.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <cstdint>
#include <string>

namespace engine {

struct QuitRequested {};

struct WindowResized {
    uint32_t width = 0, height = 0;
};

struct WindowFocusChanged {
    bool focused = true;
};

struct AppSuspended {};

struct AppResumed {};

struct Collision {
    Entity a, b;
};

struct SceneLoaded {
    std::string name;
};

struct SceneUnloading {
    std::string name;
};

struct Caption {
    std::string text;
    float seconds = 0;
};

struct ConsoleCommand {
    std::string line;
};

}

ENGINE_EVENT(engine::QuitRequested)
ENGINE_EVENT(engine::WindowResized)
ENGINE_EVENT(engine::WindowFocusChanged)
ENGINE_EVENT(engine::AppSuspended)
ENGINE_EVENT(engine::AppResumed)
ENGINE_EVENT(engine::Collision)
ENGINE_EVENT(engine::SceneLoaded)
ENGINE_EVENT(engine::SceneUnloading)
ENGINE_EVENT(engine::Caption)
ENGINE_EVENT(engine::ConsoleCommand)
