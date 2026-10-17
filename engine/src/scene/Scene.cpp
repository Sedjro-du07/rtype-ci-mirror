/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Scene
*/

#include "engine/scene/Scene.hpp"

#include <utility>

namespace engine {

Scene::Scene(const TypeRegistry& types, std::string name, std::filesystem::path path)
    : _name(std::move(name)), _path(std::move(path)), _registry(types) {}

const std::string& Scene::name() const {
    return this->_name;
}

const std::filesystem::path& Scene::path() const {
    return this->_path;
}

Registry& Scene::registry() {
    return this->_registry;
}

const Registry& Scene::registry() const {
    return this->_registry;
}

void Scene::setUuid(Entity e, const Uuid& uuid) {
    this->_uuids[e] = uuid;
}

std::optional<Uuid> Scene::uuidOf(Entity e) const {
    const auto it = this->_uuids.find(e);
    if (it == this->_uuids.end()) return std::nullopt;
    return it->second;
}

void Scene::setPrefab(Entity e, std::string path) {
    this->_prefabs[e] = std::move(path);
}

std::string_view Scene::prefabOf(Entity e) const {
    const auto it = this->_prefabs.find(e);
    return it == this->_prefabs.end() ? std::string_view{} : std::string_view(it->second);
}

void Scene::markGenerated(Entity e) {
    this->_generated.insert(e);
}

bool Scene::generated(Entity e) const {
    return this->_generated.contains(e);
}

}
