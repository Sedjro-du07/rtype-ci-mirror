/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Action
*/

#pragma once

#include "engine/core/Hash.hpp"
#include "engine/input/Action.hpp"

#include <cstddef>
#include <string_view>

namespace engine {

constexpr ActionId action(std::string_view name) noexcept {
    return ActionId{fnv1a64(name)};
}

}

constexpr size_t std::hash<engine::ActionId>::operator()(const engine::ActionId& id) const noexcept {
    return static_cast<size_t>(id.key ^ (id.key >> 32));
}
