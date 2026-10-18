/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** NetworkIdMap
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/ecs/Entity.hpp"

#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace engine {

class ENGINE_API NetworkIdMap {
private:
    std::unordered_map<uint32_t, Entity> _entities;
    uint32_t _next = 1;

public:
    uint32_t allocate();
    void bind(uint32_t id, Entity e);
    void unbind(uint32_t id);
    Entity find(uint32_t id) const;
    size_t size() const;
    void clear();
};

}
