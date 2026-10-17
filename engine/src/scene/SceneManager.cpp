/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SceneManager
*/

#include "engine/scene/SceneManager.hpp"

#include "engine/components/Hierarchy.hpp"
#include "engine/events/EngineEvents.hpp"
#include "engine/events/EventBus.hpp"
#include "engine/reflection/TypeRegistry.hpp"

#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

namespace engine {

namespace {

void collect(const Registry& registry, Entity e, std::vector<Entity>& out) {
    if (!registry.alive(e) || std::ranges::contains(out, e)) return;
    out.push_back(e);
    for (Entity child : registry.children(e))
        collect(registry, child, out);
}

}

SceneManager::SceneManager(const TypeRegistry& types, AssetManager& assets, EventBus& events, Logger& log)
    : _types(&types), _events(&events), _log(log.tagged("scene")), _prefabs(types, assets),
      _loader(types, assets, _prefabs), _active(std::make_unique<Scene>(types, "", "")) {}

void SceneManager::carryPersistent(Scene& next) {
    Registry& from = this->_active->registry();
    Registry& to = next.registry();
    std::vector<Entity> moving;
    for (Entity e : this->_persistent)
        collect(from, e, moving);

    const TypeInfo* parentType = this->_types->find("Parent");
    const TypeInfo* childrenType = this->_types->find("Children");
    std::unordered_map<Entity, Entity> copies;
    for (Entity old : moving) {
        const Entity copy = to.create();
        copies.emplace(old, copy);
        for (ComponentId id : from.componentsOf(old)) {
            if ((parentType && id == parentType->id) || (childrenType && id == childrenType->id)) continue;
            to.emplaceRaw(copy, id, from.getRaw(old, id));
        }
        if (auto uuid = this->_active->uuidOf(old)) next.setUuid(copy, *uuid);
        if (const auto prefab = this->_active->prefabOf(old); !prefab.empty())
            next.setPrefab(copy, std::string(prefab));
        if (this->_active->generated(old)) next.markGenerated(copy);
    }
    if (parentType)
        for (Entity old : moving) {
            const auto* parent = static_cast<const Parent*>(from.getRaw(old, parentType->id));
            if (!parent) continue;
            const auto target = copies.find(parent->value);
            if (target != copies.end()) to.setParent(copies.at(old), target->second, parent->offset);
        }

    std::vector<Entity> kept;
    for (Entity e : this->_persistent)
        if (const auto it = copies.find(e); it != copies.end()) kept.push_back(it->second);
    this->_persistent = std::move(kept);
}

void SceneManager::activate(std::unique_ptr<Scene> next) {
    if (!this->_active->name().empty() || !this->_active->path().empty())
        this->_events->emit(SceneUnloading{this->_active->name()});
    this->carryPersistent(*next);
    this->_active = std::move(next);
    this->_log.info("scene '{}' loaded from {}", this->_active->name(), this->_active->path().generic_string());
    this->_events->emit(SceneLoaded{this->_active->name()});
}

Result<void> SceneManager::load(const std::filesystem::path& file) {
    auto next = this->_loader.load(file);
    if (!next) {
        this->_log.error("{}", next.error().toString());
        return unexpected(next.error());
    }
    this->activate(std::move(*next));
    return {};
}

void SceneManager::switchTo(const std::filesystem::path& file) {
    this->_pending = file;
}

void SceneManager::markPersistent(Entity e) {
    if (!std::ranges::contains(this->_persistent, e)) this->_persistent.push_back(e);
}

Result<void> SceneManager::save(const std::filesystem::path& file) const {
    return this->_loader.save(*this->_active, file);
}

Registry& SceneManager::world() {
    return this->_active->registry();
}

const Scene& SceneManager::active() const {
    return *this->_active;
}

std::string_view SceneManager::activeName() const {
    return this->_active->name();
}

PrefabLibrary& SceneManager::prefabs() {
    return this->_prefabs;
}

bool SceneManager::hasPending() const {
    return this->_pending.has_value();
}

void SceneManager::applyPending() {
    if (!this->_pending) return;
    const std::filesystem::path file = std::move(*this->_pending);
    this->_pending.reset();
    auto next = this->_loader.load(file);
    if (!next) {
        this->_log.error("cannot switch to '{}', staying on '{}': {}", file.generic_string(), this->_active->name(),
                         next.error().toString());
        return;
    }
    this->activate(std::move(*next));
}

void SceneManager::clear() {
    this->_pending.reset();
    this->_persistent.clear();
    this->_active = std::make_unique<Scene>(*this->_types, "", "");
    this->_prefabs.clear();
}

}
