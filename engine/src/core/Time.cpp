/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Time
*/

#include "engine/core/Time.hpp"

namespace engine {

using Seconds = std::chrono::duration<double>;

Clock::Clock() : start_(std::chrono::steady_clock::now()) {}

double Clock::restart() {
    const auto now = std::chrono::steady_clock::now();
    const double seconds = Seconds(now - start_).count();
    start_ = now;
    return seconds;
}

double Clock::elapsed() const {
    return Seconds(std::chrono::steady_clock::now() - start_).count();
}

}
