/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EntityAllocator
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/ecs/Entity.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace engine {
class ENGINE_API EntityAllocator {
private:
    std::vector<uint32_t> _generations;
    std::vector<uint32_t> _freeNumbers;
    std::size_t _aliveEntities;

public:
    EntityAllocator();
    ~EntityAllocator() = default;
    Entity create();
    void release(Entity e);
    bool alive(Entity e) const;
    size_t aliveCount() const;
};
}
