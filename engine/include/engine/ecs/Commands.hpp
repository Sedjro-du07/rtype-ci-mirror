/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Commands
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/IStorage.hpp"
#include "engine/ecs/Registry.hpp"

#include <cstddef>
#include <functional>
#include <string_view>
#include <vector>

namespace engine {

class PrefabLibrary;

class ENGINE_API Commands {
private:
    using Command = std::move_only_function<void(Registry&)>;

    Registry* _registry;
    PrefabLibrary* _prefabs;
    std::vector<Command> _queue;

public:
    explicit Commands(Registry& registry, PrefabLibrary* prefabs = nullptr);
    Commands(const Commands&) = delete;
    Commands& operator=(const Commands&) = delete;

    Entity spawn();
    Entity spawn(std::string_view prefab, Vec2 position);
    void destroy(Entity e);
    template <class T>
    void add(Entity e, T value);
    template <class T>
    void set(Entity e, T value);
    template <class T>
    void remove(Entity e);
    void setParent(Entity child, Entity parent, Vec2 localOffset = {});
    void addRaw(Entity e, ComponentId id, std::vector<std::byte> value);
    bool empty() const;
    void apply();
};

}

#include "engine/ecs/Commands.inl"
