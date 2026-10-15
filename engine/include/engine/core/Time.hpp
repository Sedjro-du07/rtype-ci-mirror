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

// Filled by the engine every frame (fixed-step loop, ADR 0003); systems read it through ctx.time().
// Simulation uses fixedDelta, never frameDelta: the game must run at the same speed on every machine.
struct Time {
    double fixedDelta = 1.0 / 60.0; // seconds per FixedUpdate step; the dt every simulation system uses
    uint64_t tick = 0;              // number of fixed steps since start
    double alpha = 0.0;             // [0,1): position between the last two ticks, for interpolation
    double total = 0.0;             // seconds of simulated time since start
    double frameDelta = 0.0;        // real seconds of the last frame (rendering, UI only)
};

// Real time, measured on steady_clock: never system_clock, which jumps with NTP or a clock change.
class ENGINE_API Clock {
public:
    Clock();                // starts measuring
    double restart();       // seconds since construction or last restart, then resets
    double elapsed() const; // seconds since last restart, without resetting

private:
    std::chrono::steady_clock::time_point start_;
};

} // namespace engine
