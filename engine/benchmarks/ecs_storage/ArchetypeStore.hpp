/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ArchetypeStore
*/

#pragma once

#include "BenchComponents.hpp"
#include "engine/ecs/Entity.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace engine::bench {

enum ComponentBit : uint8_t { PositionBit = 1, VelocityBit = 2, HealthBit = 4 };

struct ArchetypeTable {
    std::vector<Entity> entities;
    std::vector<Position> positions;
    std::vector<Velocity> velocities;
    std::vector<Health> healths;
};

class MinimalArchetypeStore {
private:
    struct Location {
        uint8_t mask = 0;
        uint32_t row = 0;
    };

    std::array<ArchetypeTable, 8> _tables;
    std::vector<Location> _locations;

    void moveTo(Entity e, uint8_t mask);

public:
    void create(Entity e, uint8_t mask);
    void add(Entity e, uint8_t bit);
    void remove(Entity e, uint8_t bit);
    ArchetypeTable& table(uint8_t mask);
};

}
