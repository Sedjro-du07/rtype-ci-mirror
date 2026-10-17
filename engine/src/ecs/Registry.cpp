/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Registry
*/

#include "engine/ecs/Registry.hpp"

#include "engine/core/Hash.hpp"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>

namespace engine {

namespace detail {

void unregisteredComponent(std::string_view name) {
    const std::string text(name);
    std::fprintf(stderr,
                 "engine: component '%s' is not registered: call types.add<%s>() in registerTypes() before using it\n",
                 text.c_str(), text.c_str());
    std::fflush(stderr);
    std::_Exit(3);
}

}

Registry::Registry(const TypeRegistry& types) : _types(&types) {
    const TypeInfo* uuid = this->registered("Uuid");
    if (!uuid) return;
    ComponentHooks& hooks = this->hooks(uuid->id);
    this->_uuidIndex.push_back(hooks.onCreate.connect([this](Registry&, Entity e) { this->indexUuid(e); }));
    this->_uuidIndex.push_back(hooks.onReplace.connect([this](Registry&, Entity e) { this->indexUuid(e); }));
    this->_uuidIndex.push_back(hooks.onDestroy.connect([this](Registry&, Entity e) { this->unindexUuid(e); }));
}

const TypeRegistry& Registry::types() const {
    return *this->_types;
}

Entity Registry::create() {
    return this->_entities.create();
}

void Registry::destroy(Entity e) {
    if (!this->alive(e)) return;
    const std::span<const Entity> kids = this->children(e);
    const std::vector<Entity> copy(kids.begin(), kids.end());
    for (Entity child : copy)
        this->destroy(child);
    this->detachFromParent(e);
    for (ComponentId id = 0; id < this->_storages.size(); ++id)
        this->removeRaw(e, id);
    this->_entities.release(e);
}

bool Registry::alive(Entity e) const {
    return this->_entities.alive(e);
}

size_t Registry::aliveCount() const {
    return this->_entities.aliveCount();
}

void Registry::clear() {
    for (ComponentId id = 0; id < this->_storages.size(); ++id) {
        IStorage* set = this->_storages[id].get();
        if (!set) continue;
        const std::span<const Entity> owners = set->entities();
        const std::vector<Entity> copy(owners.begin(), owners.end());
        for (Entity e : copy)
            this->removeRaw(e, id);
    }
    this->_entities = EntityAllocator{};
    this->_singletons = Entity::null();
}

IStorage* Registry::storage(ComponentId id) {
    return id < this->_storages.size() ? this->_storages[id].get() : nullptr;
}

const IStorage* Registry::storage(ComponentId id) const {
    return id < this->_storages.size() ? this->_storages[id].get() : nullptr;
}

IStorage& Registry::assure(ComponentId id) {
    if (id >= this->_storages.size()) this->_storages.resize(id + 1);
    if (!this->_storages[id]) {
        const TypeInfo& info = this->_types->info(id);
        assert(info.category == TypeCategory::Component && info.makeStorage);
        this->_storages[id] = info.makeStorage(id);
    }
    return *this->_storages[id];
}

void* Registry::getRaw(Entity e, ComponentId id) {
    IStorage* set = this->storage(id);
    return set ? set->getRaw(e) : nullptr;
}

const void* Registry::getRaw(Entity e, ComponentId id) const {
    const IStorage* set = this->storage(id);
    return set ? set->getRaw(e) : nullptr;
}

void* Registry::emplaceRaw(Entity e, ComponentId id, const void* src) {
    assert(this->alive(e));
    this->assure(id).emplaceRaw(e, src);
    this->fire(&ComponentHooks::onCreate, id, e);
    return this->getRaw(e, id);
}

void Registry::removeRaw(Entity e, ComponentId id) {
    IStorage* set = this->storage(id);
    if (!set || !set->has(e)) return;
    this->fire(&ComponentHooks::onDestroy, id, e);
    set = this->storage(id);
    if (set) set->remove(e);
}

std::vector<ComponentId> Registry::componentsOf(Entity e) const {
    std::vector<ComponentId> ids;
    for (ComponentId id = 0; id < this->_storages.size(); ++id)
        if (this->_storages[id] && this->_storages[id]->has(e)) ids.push_back(id);
    return ids;
}

std::vector<Entity> Registry::entities() const {
    std::vector<Entity> found;
    for (const auto& set : this->_storages)
        if (set)
            for (Entity e : set->entities())
                if (e != this->_singletons && this->alive(e)) found.push_back(e);
    std::ranges::sort(found, {}, &Entity::index);
    const auto [first, last] = std::ranges::unique(found);
    found.erase(first, last);
    return found;
}

const TypeInfo* Registry::registered(std::string_view name) const {
    const TypeInfo* info = this->_types->find(name);
    return info && info->category == TypeCategory::Component ? info : nullptr;
}

Entity Registry::find(std::string_view name) const {
    const TypeInfo* info = this->registered("Name");
    const IStorage* set = info ? this->storage(info->id) : nullptr;
    if (!set) return Entity::null();
    for (Entity e : set->entities())
        if (static_cast<const Name*>(set->getRaw(e))->value == name) return e;
    return Entity::null();
}

Entity Registry::find(const Uuid& uuid) const {
    const auto it = this->_byUuid.find(uuid);
    return it == this->_byUuid.end() ? Entity::null() : it->second;
}

std::vector<Entity> Registry::withTag(std::string_view tag) const {
    std::vector<Entity> found;
    const TypeInfo* info = this->registered("Tags");
    const IStorage* set = info ? this->storage(info->id) : nullptr;
    if (!set) return found;
    for (Entity e : set->entities())
        if (static_cast<const Tags*>(set->getRaw(e))->has(tag)) found.push_back(e);
    return found;
}

void Registry::setParent(Entity child, Entity parent, Vec2 localOffset) {
    assert(this->alive(child));
    assert(parent.isNull() || (this->alive(parent) && parent != child && !this->isAncestor(child, parent)));
    this->detachFromParent(child);
    if (parent.isNull()) {
        this->remove<Parent>(child);
        return;
    }
    if (this->has<Parent>(child))
        this->replace<Parent>(child, Parent{parent, localOffset});
    else
        this->emplace<Parent>(child, Parent{parent, localOffset});
    if (!this->has<Children>(parent)) this->emplace<Children>(parent);
    this->get<Children>(parent).list.push_back(child);
}

Entity Registry::parent(Entity e) const {
    const TypeInfo* info = this->registered("Parent");
    const IStorage* set = info ? this->storage(info->id) : nullptr;
    if (!set || !set->has(e)) return Entity::null();
    return static_cast<const Parent*>(set->getRaw(e))->value;
}

std::span<const Entity> Registry::children(Entity e) const {
    const TypeInfo* info = this->registered("Children");
    const IStorage* set = info ? this->storage(info->id) : nullptr;
    if (!set || !set->has(e)) return {};
    return static_cast<const Children*>(set->getRaw(e))->list;
}

ComponentHooks& Registry::hooks(ComponentId id) {
    if (id >= this->_hooks.size()) this->_hooks.resize(id + 1);
    if (!this->_hooks[id]) this->_hooks[id] = std::make_unique<ComponentHooks>();
    return *this->_hooks[id];
}

void Registry::fire(Hook hook, ComponentId id, Entity e) {
    if (id >= this->_hooks.size() || !this->_hooks[id]) return;
    (this->_hooks[id].get()->*hook).emit(*this, e);
}

void Registry::indexUuid(Entity e) {
    this->unindexUuid(e);
    const Uuid& uuid = this->get<Uuid>(e);
    this->_byUuid[uuid] = e;
    this->_uuidOf[e] = uuid;
}

void Registry::unindexUuid(Entity e) {
    const auto it = this->_uuidOf.find(e);
    if (it == this->_uuidOf.end()) return;
    this->_byUuid.erase(it->second);
    this->_uuidOf.erase(it);
}

void Registry::detachFromParent(Entity child) {
    const Entity old = this->parent(child);
    if (old.isNull() || !this->alive(old)) return;
    const TypeInfo* info = this->registered("Children");
    IStorage* set = info ? this->storage(info->id) : nullptr;
    if (set && set->has(old)) std::erase(static_cast<Children*>(set->getRaw(old))->list, child);
}

bool Registry::isAncestor(Entity ancestor, Entity e) const {
    for (Entity current = this->parent(e); !current.isNull(); current = this->parent(current))
        if (current == ancestor) return true;
    return false;
}

}
