/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Action
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>

namespace engine {

struct ActionId {
    uint64_t key = 0;

    constexpr bool operator==(const ActionId&) const = default;
};

constexpr ActionId action(std::string_view name) noexcept;

}

template <>
struct std::hash<engine::ActionId> {
    constexpr size_t operator()(const engine::ActionId& id) const noexcept;
};

#include "engine/input/Action.inl"
