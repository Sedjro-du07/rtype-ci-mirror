/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** WindowDesc
*/

#pragma once

#include <cstdint>
#include <string>

namespace engine {

struct WindowDesc {
    std::string title = "Engine";
    uint32_t width = 1280, height = 720;
    uint32_t virtualWidth = 1280, virtualHeight = 720;
    bool fullscreen = false;
    bool vsync = true;
    float uiScale = 1.f;
};

}
