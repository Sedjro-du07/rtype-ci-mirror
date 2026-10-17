/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ActionState
*/

#include "engine/input/ActionState.hpp"

#include <algorithm>
#include <cstddef>

namespace engine {

const ActionState::Entry* ActionState::find(ActionId a) const {
    const auto it = this->_actions.find(a);
    return it == this->_actions.end() ? nullptr : &it->second;
}

bool ActionState::pressed(ActionId a) const {
    const Entry* entry = this->find(a);
    return entry && entry->pressed;
}

bool ActionState::held(ActionId a) const {
    const Entry* entry = this->find(a);
    return entry && entry->held;
}

bool ActionState::released(ActionId a) const {
    const Entry* entry = this->find(a);
    return entry && entry->released;
}

float ActionState::axis(ActionId a) const {
    const Entry* entry = this->find(a);
    return entry ? entry->axis : 0.f;
}

uint64_t ActionState::toMask(std::span<const ActionId> order) const {
    uint64_t mask = 0;
    for (size_t i = 0; i < std::min<size_t>(order.size(), 64); ++i)
        if (this->held(order[i])) mask |= uint64_t{1} << i;
    return mask;
}

ActionState ActionState::fromMask(uint64_t mask, uint64_t previousMask, std::span<const ActionId> order) {
    ActionState state;
    for (size_t i = 0; i < std::min<size_t>(order.size(), 64); ++i) {
        const bool now = (mask >> i) & 1;
        const bool before = (previousMask >> i) & 1;
        state._actions[order[i]] = Entry{now, now && !before, !now && before, now ? 1.f : 0.f};
    }
    return state;
}

}
