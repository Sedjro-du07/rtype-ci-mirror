/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Stage
*/

#pragma once

#include <cstddef>
#include <cstdint>

namespace engine {

enum class Stage : uint8_t { FixedUpdate, LateUpdate, Render };

inline constexpr size_t StageCount = 3;

}
