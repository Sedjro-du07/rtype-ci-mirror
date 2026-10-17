/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Time
*/

#pragma once

#include "engine/Export.hpp"

#include <chrono>
#include <cstdint>

namespace engine {

struct Time {
    double fixedDelta = 1.0 / 60.0;
    uint64_t tick = 0;
    double alpha = 0.0;
    double total = 0.0;
    double frameDelta = 0.0;
};

class ENGINE_API Clock {
private:
    std::chrono::steady_clock::time_point start_;

public:
    Clock();
    double restart();
    double elapsed() const;
};

}
