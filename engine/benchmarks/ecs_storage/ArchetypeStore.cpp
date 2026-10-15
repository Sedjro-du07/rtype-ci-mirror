/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ArchetypeStore
*/

#include "ArchetypeStore.hpp"

#include <cstddef>

namespace engine::bench {

void MinimalArchetypeStore::create(Entity e, uint8_t mask) {
    if (e.index >= this->_locations.size()) this->_locations.resize(e.index + 1);
    ArchetypeTable& table = this->_tables[mask];
    this->_locations[e.index] = {mask, static_cast<uint32_t>(table.entities.size())};
    table.entities.push_back(e);
    if (mask & PositionBit) table.positions.emplace_back();
    if (mask & VelocityBit) table.velocities.emplace_back();
    if (mask & HealthBit) table.healths.emplace_back();
}

void MinimalArchetypeStore::add(Entity e, uint8_t bit) {
    this->moveTo(e, static_cast<uint8_t>(this->_locations[e.index].mask | bit));
}

void MinimalArchetypeStore::remove(Entity e, uint8_t bit) {
    this->moveTo(e, static_cast<uint8_t>(this->_locations[e.index].mask & ~bit));
}

ArchetypeTable& MinimalArchetypeStore::table(uint8_t mask) {
    return this->_tables[mask];
}

void MinimalArchetypeStore::moveTo(Entity e, uint8_t mask) {
    Location& from = this->_locations[e.index];
    if (from.mask == mask) return;
    ArchetypeTable& src = this->_tables[from.mask];
    ArchetypeTable& dst = this->_tables[mask];
    const uint32_t row = from.row;

    dst.entities.push_back(e);
    if (mask & PositionBit) dst.positions.push_back(from.mask & PositionBit ? src.positions[row] : Position{});
    if (mask & VelocityBit) dst.velocities.push_back(from.mask & VelocityBit ? src.velocities[row] : Velocity{});
    if (mask & HealthBit) dst.healths.push_back(from.mask & HealthBit ? src.healths[row] : Health{});

    const uint32_t last = static_cast<uint32_t>(src.entities.size() - 1);
    if (row != last) {
        const Entity moved = src.entities[last];
        src.entities[row] = moved;
        if (from.mask & PositionBit) src.positions[row] = src.positions[last];
        if (from.mask & VelocityBit) src.velocities[row] = src.velocities[last];
        if (from.mask & HealthBit) src.healths[row] = src.healths[last];
        this->_locations[moved.index].row = row;
    }
    src.entities.pop_back();
    if (from.mask & PositionBit) src.positions.pop_back();
    if (from.mask & VelocityBit) src.velocities.pop_back();
    if (from.mask & HealthBit) src.healths.pop_back();

    from = {mask, static_cast<uint32_t>(dst.entities.size() - 1)};
}

}
