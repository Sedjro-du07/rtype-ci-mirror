/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EntityAllocator
*/

#include "engine/ecs/EntityAllocator.hpp"

#include <cstddef>
#include <cstdint>

namespace engine {
EntityAllocator::EntityAllocator() {
    this->_aliveEntities = 0;
}

Entity EntityAllocator::create() {
    std::uint32_t nextIndex = 0;

    if (this->_freeNumbers.size() == 0) {
        nextIndex = static_cast<std::uint32_t>(this->_generations.size());
        this->_aliveEntities++;
        this->_generations.push_back(0);
        return Entity{nextIndex, 0};
    }
    nextIndex = this->_freeNumbers.back();
    this->_freeNumbers.pop_back();
    this->_aliveEntities++;
    return Entity{nextIndex, this->_generations[nextIndex]};
}

void EntityAllocator::release(Entity e) {
    if (!this->alive(e)) return;
    this->_generations[e.index]++;
    this->_freeNumbers.push_back(e.index);
    this->_aliveEntities--;
}

bool EntityAllocator::alive(Entity e) const {
    return e.index < this->_generations.size() && this->_generations[e.index] == e.generation;
}

size_t EntityAllocator::aliveCount() const {
    return this->_aliveEntities;
}
}
