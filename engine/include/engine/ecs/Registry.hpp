/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Registry
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/components/Hierarchy.hpp"
#include "engine/components/Identity.hpp"
#include "engine/core/Signal.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/EntityAllocator.hpp"
#include "engine/ecs/IStorage.hpp"
#include "engine/ecs/SparseSet.hpp"
#include "engine/ecs/View.hpp"
#include "engine/reflection/TypeRegistry.hpp"

#include <cstddef>
#include <memory>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace engine {

class Registry;

struct ComponentHooks {
    Signal<void(Registry&, Entity)> onCreate;
    Signal<void(Registry&, Entity)> onReplace;
    Signal<void(Registry&, Entity)> onDestroy;
};

namespace detail {

[[noreturn]] ENGINE_API void unregisteredComponent(std::string_view name);

}

class ENGINE_API Registry {
private:
    using Hook = Signal<void(Registry&, Entity)> ComponentHooks::*;

    const TypeRegistry* _types;
    EntityAllocator _entities;
    std::vector<std::unique_ptr<IStorage>> _storages;
    std::vector<std::unique_ptr<ComponentHooks>> _hooks;
    std::unordered_map<Uuid, Entity> _byUuid;
    std::unordered_map<Entity, Uuid> _uuidOf;
    std::vector<Subscription> _uuidIndex;
    Entity _singletons;

    template <class T>
    ComponentId componentId() const;
    template <class T>
    SparseSet<T>& assure();
    template <class T>
    SparseSet<T>* existing() const;

    IStorage& assure(ComponentId id);
    const IStorage* storage(ComponentId id) const;
    const TypeInfo* registered(std::string_view name) const;
    void fire(Hook hook, ComponentId id, Entity e);
    void indexUuid(Entity e);
    void unindexUuid(Entity e);
    void detachFromParent(Entity child);
    bool isAncestor(Entity ancestor, Entity e) const;

public:
    explicit Registry(const TypeRegistry& types);
    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;

    const TypeRegistry& types() const;

    Entity create();
    void destroy(Entity e);
    bool alive(Entity e) const;
    size_t aliveCount() const;
    void clear();

    template <class T, class... Args>
    T& emplace(Entity e, Args&&... args);
    template <class T>
    T& get(Entity e);
    template <class T>
    const T& get(Entity e) const;
    template <class T>
    T* tryGet(Entity e);
    template <class T>
    bool has(Entity e) const;
    template <class T>
    T& replace(Entity e, T value);
    template <class T>
    void remove(Entity e);

    template <class... Ts>
    View<Ts...> view();
    template <class... Ts, class... Xs>
    View<Ts...> view(Exclude<Xs...>);

    IStorage* storage(ComponentId id);
    void* getRaw(Entity e, ComponentId id);
    void* emplaceRaw(Entity e, ComponentId id, const void* src);
    void removeRaw(Entity e, ComponentId id);
    std::vector<ComponentId> componentsOf(Entity e) const;

    Entity find(std::string_view name) const;
    Entity find(const Uuid& uuid) const;
    std::vector<Entity> withTag(std::string_view tag) const;
    void setParent(Entity child, Entity parent, Vec2 localOffset = {});
    Entity parent(Entity e) const;
    std::span<const Entity> children(Entity e) const;

    template <class T>
    T& singleton();
    template <class T>
    T* trySingleton();

    template <class T>
    ComponentHooks& hooks();
    ComponentHooks& hooks(ComponentId id);
};

}

#include "engine/ecs/Registry.inl"
