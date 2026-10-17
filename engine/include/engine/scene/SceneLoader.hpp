/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SceneLoader
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Error.hpp"
#include "engine/scene/Prefab.hpp"
#include "engine/scene/Scene.hpp"

#include <filesystem>
#include <memory>

namespace engine {

class AssetManager;
class TypeRegistry;

class ENGINE_API SceneLoader {
private:
    const TypeRegistry* _types;
    AssetManager* _assets;
    PrefabLibrary* _prefabs;

public:
    SceneLoader(const TypeRegistry& types, AssetManager& assets, PrefabLibrary& prefabs);

    Result<std::unique_ptr<Scene>> load(const std::filesystem::path& file);
    Result<void> save(const Scene& scene, const std::filesystem::path& file) const;
};

}
