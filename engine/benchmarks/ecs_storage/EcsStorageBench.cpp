/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EcsStorageBench
*/

#include "ArchetypeStore.hpp"
#include "BenchComponents.hpp"
#include "SparseArray.hpp"
#include "engine/ecs/SparseSet.hpp"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace {

using namespace engine;
using namespace engine::bench;

constexpr uint32_t EntityCount = 10000;
constexpr float Dt = 1.f / 60.f;

Entity entityAt(uint32_t i, uint32_t spread) {
    return Entity{i * spread, 0};
}

bool hasVelocity(uint32_t i) {
    return i % 2 == 0;
}

bool hasHealth(uint32_t i) {
    return i % 3 == 0;
}

struct SparseSetWorld {
    SparseSet<Position> positions{0};
    SparseSet<Velocity> velocities{1};
    SparseSet<Health> healths{2};

    void build(uint32_t spread, bool withHealth) {
        for (uint32_t i = 0; i < EntityCount; ++i) {
            const Entity e = entityAt(i, spread);
            this->positions.emplace(e);
            if (hasVelocity(i)) this->velocities.emplace(e);
            if (withHealth && hasHealth(i)) this->healths.emplace(e);
        }
    }

    void iterate1() {
        for (Position& p : this->positions.components())
            p.x += 1.f;
    }

    void iterate2() {
        const auto entities = this->velocities.entities();
        const auto velocities = this->velocities.components();
        for (size_t i = 0; i < entities.size(); ++i) {
            Position& p = this->positions.get(entities[i]);
            p.x += velocities[i].x * Dt;
            p.y += velocities[i].y * Dt;
        }
    }

    void iterate3() {
        const auto entities = this->healths.entities();
        const auto healths = this->healths.components();
        for (size_t i = 0; i < entities.size(); ++i) {
            Velocity* v = this->velocities.tryGet(entities[i]);
            if (!v || healths[i].value <= 0) continue;
            Position& p = this->positions.get(entities[i]);
            p.x += v->x * Dt;
            p.y += v->y * Dt;
        }
    }

    void addRemove(uint32_t spread) {
        for (uint32_t i = 0; i < EntityCount; ++i)
            this->healths.emplace(entityAt(i, spread));
        for (uint32_t i = 0; i < EntityCount; ++i)
            this->healths.remove(entityAt(i, spread));
    }
};

struct SparseArrayWorld {
    SparseArray<Position> positions;
    SparseArray<Velocity> velocities;
    SparseArray<Health> healths;

    void build(uint32_t spread, bool withHealth) {
        for (uint32_t i = 0; i < EntityCount; ++i) {
            const Entity e = entityAt(i, spread);
            this->positions.emplace(e);
            if (hasVelocity(i)) this->velocities.emplace(e);
            if (withHealth && hasHealth(i)) this->healths.emplace(e);
        }
    }

    void iterate1() {
        for (auto& slot : this->positions.slots())
            if (slot) slot->x += 1.f;
    }

    void iterate2() {
        auto positions = this->positions.slots();
        auto velocities = this->velocities.slots();
        const size_t count = std::min(positions.size(), velocities.size());
        for (size_t i = 0; i < count; ++i) {
            if (!positions[i] || !velocities[i]) continue;
            positions[i]->x += velocities[i]->x * Dt;
            positions[i]->y += velocities[i]->y * Dt;
        }
    }

    void iterate3() {
        auto positions = this->positions.slots();
        auto velocities = this->velocities.slots();
        auto healths = this->healths.slots();
        const size_t count = std::min({positions.size(), velocities.size(), healths.size()});
        for (size_t i = 0; i < count; ++i) {
            if (!positions[i] || !velocities[i] || !healths[i] || healths[i]->value <= 0) continue;
            positions[i]->x += velocities[i]->x * Dt;
            positions[i]->y += velocities[i]->y * Dt;
        }
    }

    void addRemove(uint32_t spread) {
        for (uint32_t i = 0; i < EntityCount; ++i)
            this->healths.emplace(entityAt(i, spread));
        for (uint32_t i = 0; i < EntityCount; ++i)
            this->healths.remove(entityAt(i, spread));
    }
};

struct ArchetypeWorld {
    MinimalArchetypeStore store;

    void build(uint32_t spread, bool withHealth) {
        for (uint32_t i = 0; i < EntityCount; ++i) {
            uint8_t mask = PositionBit;
            if (hasVelocity(i)) mask |= VelocityBit;
            if (withHealth && hasHealth(i)) mask |= HealthBit;
            this->store.create(entityAt(i, spread), mask);
        }
    }

    void iterate1() {
        for (uint8_t mask = 0; mask < 8; ++mask) {
            if (!(mask & PositionBit)) continue;
            for (Position& p : this->store.table(mask).positions)
                p.x += 1.f;
        }
    }

    void iterate2() {
        for (uint8_t mask = 0; mask < 8; ++mask) {
            if ((mask & (PositionBit | VelocityBit)) != (PositionBit | VelocityBit)) continue;
            ArchetypeTable& table = this->store.table(mask);
            for (size_t i = 0; i < table.entities.size(); ++i) {
                table.positions[i].x += table.velocities[i].x * Dt;
                table.positions[i].y += table.velocities[i].y * Dt;
            }
        }
    }

    void iterate3() {
        ArchetypeTable& table = this->store.table(PositionBit | VelocityBit | HealthBit);
        for (size_t i = 0; i < table.entities.size(); ++i) {
            if (table.healths[i].value <= 0) continue;
            table.positions[i].x += table.velocities[i].x * Dt;
            table.positions[i].y += table.velocities[i].y * Dt;
        }
    }

    void addRemove(uint32_t spread) {
        for (uint32_t i = 0; i < EntityCount; ++i)
            this->store.add(entityAt(i, spread), HealthBit);
        for (uint32_t i = 0; i < EntityCount; ++i)
            this->store.remove(entityAt(i, spread), HealthBit);
    }
};

template <class World>
void benchIterate1(benchmark::State& state) {
    World world;
    world.build(static_cast<uint32_t>(state.range(0)), true);
    for (auto _ : state) {
        world.iterate1();
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * EntityCount);
}

template <class World>
void benchIterate2(benchmark::State& state) {
    World world;
    world.build(static_cast<uint32_t>(state.range(0)), true);
    for (auto _ : state) {
        world.iterate2();
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * EntityCount / 2);
}

template <class World>
void benchIterate3(benchmark::State& state) {
    World world;
    world.build(static_cast<uint32_t>(state.range(0)), true);
    for (auto _ : state) {
        world.iterate3();
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * EntityCount / 6);
}

template <class World>
void benchAddRemove(benchmark::State& state) {
    const uint32_t spread = static_cast<uint32_t>(state.range(0));
    World world;
    world.build(spread, false);
    for (auto _ : state) {
        world.addRemove(spread);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * EntityCount * 2);
}

}

BENCHMARK_TEMPLATE(benchIterate1, SparseSetWorld)->Arg(1)->Arg(10);
BENCHMARK_TEMPLATE(benchIterate1, SparseArrayWorld)->Arg(1)->Arg(10);
BENCHMARK_TEMPLATE(benchIterate1, ArchetypeWorld)->Arg(1)->Arg(10);
BENCHMARK_TEMPLATE(benchIterate2, SparseSetWorld)->Arg(1)->Arg(10);
BENCHMARK_TEMPLATE(benchIterate2, SparseArrayWorld)->Arg(1)->Arg(10);
BENCHMARK_TEMPLATE(benchIterate2, ArchetypeWorld)->Arg(1)->Arg(10);
BENCHMARK_TEMPLATE(benchIterate3, SparseSetWorld)->Arg(1)->Arg(10);
BENCHMARK_TEMPLATE(benchIterate3, SparseArrayWorld)->Arg(1)->Arg(10);
BENCHMARK_TEMPLATE(benchIterate3, ArchetypeWorld)->Arg(1)->Arg(10);
BENCHMARK_TEMPLATE(benchAddRemove, SparseSetWorld)->Arg(1)->Arg(10);
BENCHMARK_TEMPLATE(benchAddRemove, SparseArrayWorld)->Arg(1)->Arg(10);
BENCHMARK_TEMPLATE(benchAddRemove, ArchetypeWorld)->Arg(1)->Arg(10);
