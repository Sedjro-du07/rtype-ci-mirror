/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SparseArray
*/

#pragma once

#include "engine/ecs/Entity.hpp"

#include <optional>
#include <span>
#include <vector>

namespace engine::bench {

template <class T>
class SparseArray {
private:
    std::vector<std::optional<T>> _slots;

public:
    template <class... Args>
    T& emplace(Entity e, Args&&... args);
    void remove(Entity e);
    bool has(Entity e) const;
    T& get(Entity e);
    std::span<std::optional<T>> slots();
};

}

#include "SparseArray.inl"
