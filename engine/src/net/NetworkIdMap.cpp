/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** NetworkIdMap
*/

#include "engine/net/NetworkIdMap.hpp"

namespace engine {

uint32_t NetworkIdMap::allocate() {
    return this->_next++;
}

void NetworkIdMap::bind(uint32_t id, Entity e) {
    if (id == 0) return;
    this->_entities[id] = e;
}

void NetworkIdMap::unbind(uint32_t id) {
    this->_entities.erase(id);
}

Entity NetworkIdMap::find(uint32_t id) const {
    const auto it = this->_entities.find(id);
    return it == this->_entities.end() ? Entity::null() : it->second;
}

size_t NetworkIdMap::size() const {
    return this->_entities.size();
}

void NetworkIdMap::clear() {
    this->_entities.clear();
}

}
