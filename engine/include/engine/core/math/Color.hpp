// RGBA colour, 8 bits per channel; opaque white by default.
// ENG-04 · docs/engine/reference/01-core.md

#pragma once

#include <cstdint>

namespace engine {

struct Color {
    uint8_t r = 255, g = 255, b = 255, a = 255;

    static constexpr Color white() { return {255, 255, 255, 255}; }
    static constexpr Color black() { return {0, 0, 0, 255}; }
    static constexpr Color red() { return {255, 0, 0, 255}; }
    static constexpr Color green() { return {0, 255, 0, 255}; }
    static constexpr Color blue() { return {0, 0, 255, 255}; }
    static constexpr Color transparent() { return {0, 0, 0, 0}; }

    constexpr bool operator==(const Color&) const = default;
};

} // namespace engine
