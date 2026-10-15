// Compile-time string hash for type, event and action keys (ADR 0027).
// ENG-04 · docs/engine/reference/01-core.md

#pragma once

#include <cstdint>
#include <string_view>

namespace engine {

// FNV-1a, 64 bits. constexpr so keys can be computed in templates and macros.
constexpr uint64_t fnv1a64(std::string_view text) noexcept {
    uint64_t hash = 0xcbf29ce484222325ull;
    for (char c : text) {
        hash ^= static_cast<uint8_t>(c); // as unsigned: char may be signed
        hash *= 0x100000001b3ull;
    }
    return hash;
}

} // namespace engine
