/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SparseSet
*/

#pragma once

#include "engine/ecs/Entity.hpp"
#include "engine/ecs/IStorage.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace engine {

template <class T>
class SparseSet final : public IStorage {
private:
    static constexpr size_t PageSize = 4096;
    static constexpr uint32_t Absent = UINT32_MAX;

    using Page = std::array<uint32_t, PageSize>;

    ComponentId _id;
    std::vector<std::unique_ptr<Page>> _sparse;
    std::vector<Entity> _dense;
    std::vector<T> _data;

    uint32_t position(Entity e) const;
    uint32_t& slot(Entity e);

public:
    explicit SparseSet(ComponentId id);

    template <class... Args>
    T& emplace(Entity e, Args&&... args);
    T& get(Entity e);
    const T& get(Entity e) const;
    T* tryGet(Entity e);
    T& replace(Entity e, T value);
    std::span<T> components();

    ComponentId id() const override;
    bool has(Entity e) const override;
    void remove(Entity e) override;
    size_t size() const override;
    void clear() override;
    std::span<const Entity> entities() const override;
    void* getRaw(Entity e) override;
    const void* getRaw(Entity e) const override;
    void* emplaceRaw(Entity e, const void* src) override;
};

}

#include "engine/ecs/SparseSet.inl"
