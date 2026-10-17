/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SceneManager
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Error.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/scene/Prefab.hpp"
#include "engine/scene/Scene.hpp"
#include "engine/scene/SceneLoader.hpp"
#include "engine/services/Logger.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace engine {

class AssetManager;
class EventBus;
class TypeRegistry;

class ENGINE_API SceneManager {
private:
    const TypeRegistry* _types;
    EventBus* _events;
    Logger _log;
    PrefabLibrary _prefabs;
    SceneLoader _loader;
    std::unique_ptr<Scene> _active;
    std::optional<std::filesystem::path> _pending;
    std::vector<Entity> _persistent;

    void carryPersistent(Scene& next);
    void activate(std::unique_ptr<Scene> next);

public:
    SceneManager(const TypeRegistry& types, AssetManager& assets, EventBus& events, Logger& log);
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    Result<void> load(const std::filesystem::path& file);
    void switchTo(const std::filesystem::path& file);
    void markPersistent(Entity e);
    Result<void> save(const std::filesystem::path& file) const;
    Registry& world();
    const Scene& active() const;
    std::string_view activeName() const;
    PrefabLibrary& prefabs();
    bool hasPending() const;
    void applyPending();
    void clear();
};

}
