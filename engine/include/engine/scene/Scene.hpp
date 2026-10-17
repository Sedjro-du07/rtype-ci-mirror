/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Scene
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/components/Identity.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace engine {

class TypeRegistry;

class ENGINE_API Scene {
private:
    std::string _name;
    std::filesystem::path _path;
    Registry _registry;
    std::unordered_map<Entity, Uuid> _uuids;
    std::unordered_map<Entity, std::string> _prefabs;
    std::unordered_set<Entity> _generated;

public:
    Scene(const TypeRegistry& types, std::string name, std::filesystem::path path);
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    const std::string& name() const;
    const std::filesystem::path& path() const;
    Registry& registry();
    const Registry& registry() const;

    void setUuid(Entity e, const Uuid& uuid);
    std::optional<Uuid> uuidOf(Entity e) const;
    void setPrefab(Entity e, std::string path);
    std::string_view prefabOf(Entity e) const;
    void markGenerated(Entity e);
    bool generated(Entity e) const;
};

}
