/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Hash
*/

#pragma once

#include <cstdint>
#include <string_view>

namespace engine {

constexpr uint64_t fnv1a64(std::string_view text) noexcept;

}

#include "engine/core/Hash.inl"
