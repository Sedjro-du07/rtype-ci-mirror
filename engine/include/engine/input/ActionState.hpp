/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ActionState
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/input/Action.hpp"

#include <cstdint>
#include <span>
#include <unordered_map>

namespace engine {

class ENGINE_API ActionState {
private:
    struct Entry {
        bool held = false;
        bool pressed = false;
        bool released = false;
        float axis = 0;
    };

    std::unordered_map<ActionId, Entry> _actions;

    const Entry* find(ActionId a) const;

    friend class InputMap;

public:
    bool pressed(ActionId a) const;
    bool held(ActionId a) const;
    bool released(ActionId a) const;
    float axis(ActionId a) const;

    uint64_t toMask(std::span<const ActionId> order) const;
    static ActionState fromMask(uint64_t mask, uint64_t previousMask, std::span<const ActionId> order);
};

}
