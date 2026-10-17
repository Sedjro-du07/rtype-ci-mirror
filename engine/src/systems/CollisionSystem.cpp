/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** CollisionSystem
*/

#include "engine/components/Collider.hpp"
#include "engine/components/Position.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/events/EngineEvents.hpp"
#include "engine/events/EventBus.hpp"
#include "systems/EngineSystems.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace engine {

namespace {

struct Body {
    Entity entity;
    Rect box;
    const Collider* collider;
};

class UniformGrid {
private:
    float _cell;
    std::unordered_map<uint64_t, std::vector<size_t>> _cells;

    static uint64_t keyOf(int64_t x, int64_t y);
    int64_t cellOf(float v) const;

public:
    explicit UniformGrid(float cellSize);

    void insert(size_t body, const Rect& box);
    std::vector<std::pair<size_t, size_t>> pairs() const;
};

UniformGrid::UniformGrid(float cellSize) : _cell(cellSize > 0 ? cellSize : 64) {}

uint64_t UniformGrid::keyOf(int64_t x, int64_t y) {
    return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | static_cast<uint32_t>(y);
}

int64_t UniformGrid::cellOf(float v) const {
    return static_cast<int64_t>(std::floor(v / this->_cell));
}

void UniformGrid::insert(size_t body, const Rect& box) {
    const int64_t x0 = this->cellOf(box.x), x1 = this->cellOf(box.x + box.w);
    const int64_t y0 = this->cellOf(box.y), y1 = this->cellOf(box.y + box.h);
    for (int64_t x = x0; x <= x1; ++x)
        for (int64_t y = y0; y <= y1; ++y)
            this->_cells[keyOf(x, y)].push_back(body);
}

std::vector<std::pair<size_t, size_t>> UniformGrid::pairs() const {
    std::unordered_set<uint64_t> seen;
    std::vector<std::pair<size_t, size_t>> result;
    for (const auto& [key, bodies] : this->_cells)
        for (size_t i = 0; i < bodies.size(); ++i)
            for (size_t j = i + 1; j < bodies.size(); ++j) {
                const size_t a = std::min(bodies[i], bodies[j]);
                const size_t b = std::max(bodies[i], bodies[j]);
                if (seen.insert((static_cast<uint64_t>(a) << 32) | b).second) result.emplace_back(a, b);
            }
    std::ranges::sort(result);
    return result;
}

bool overlaps(const Rect& a, const Rect& b) {
    return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
}

}

bool layersMatch(const Collider& a, const Collider& b) {
    return (a.layer & b.mask) != 0 && (b.layer & a.mask) != 0;
}

Rect worldBox(const Position& position, const Collider& collider) {
    return {position.x + collider.offset.x, position.y + collider.offset.y, collider.size.x, collider.size.y};
}

void collision(Registry& world, EventBus& events, float cellSize) {
    std::vector<Body> bodies;
    for (auto [e, position, collider] : world.view<Position, Collider>())
        bodies.push_back(Body{e, worldBox(position, collider), &collider});
    std::ranges::sort(bodies, {}, [](const Body& body) { return body.entity.index; });

    UniformGrid grid(cellSize);
    for (size_t i = 0; i < bodies.size(); ++i)
        grid.insert(i, bodies[i].box);
    for (const auto& [i, j] : grid.pairs()) {
        const Body& a = bodies[i];
        const Body& b = bodies[j];
        if (layersMatch(*a.collider, *b.collider) && overlaps(a.box, b.box))
            events.publish(Collision{a.entity, b.entity});
    }
}

}
