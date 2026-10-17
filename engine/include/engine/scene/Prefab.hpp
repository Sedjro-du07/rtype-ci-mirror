/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Prefab
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Error.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/reflection/TypeInfo.hpp"
#include "engine/reflection/Yaml.hpp"

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace engine {

class AssetManager;
class Registry;
class TypeRegistry;

class ENGINE_API ComponentData {
private:
    const TypeInfo* _type = nullptr;
    void* _value = nullptr;
    YamlNode _source;

    void release() noexcept;

public:
    ComponentData() = default;
    explicit ComponentData(const TypeInfo& type);
    ComponentData(const ComponentData& other);
    ComponentData(ComponentData&& other) noexcept;
    ComponentData& operator=(ComponentData other) noexcept;
    ~ComponentData();

    static Result<ComponentData> parse(const TypeInfo& type, const YamlNode& node, LoadContext& ctx);
    template <class T>
    static ComponentData of(const TypeRegistry& types, T value);

    const TypeInfo& type() const;
    void* value();
    const void* value() const;
    const YamlNode& source() const;
};

struct Prefab;

struct PrefabChild {
    std::string prefab;
    const Prefab* resolved = nullptr;
    Vec2 offset;
    std::vector<ComponentData> components;
    std::vector<PrefabChild> children;
};

struct Prefab {
    std::string path;
    uint32_t key = 0;
    std::vector<ComponentData> components;
    std::vector<PrefabChild> children;
};

class ENGINE_API PrefabLibrary {
private:
    const TypeRegistry* _types;
    AssetManager* _assets;
    std::unordered_map<std::string, std::unique_ptr<Prefab>> _cache;
    std::unordered_map<uint32_t, const Prefab*> _byKey;
    std::vector<std::string> _loading;

    Result<void> parseBody(const YamlNode& node, bool child, std::vector<ComponentData>& components,
                           std::vector<PrefabChild>& children, PrefabChild* self);
    Result<PrefabChild> parseChild(const YamlNode& node);
    void build(Registry& registry, Entity e, std::span<const ComponentData> base,
               std::span<const ComponentData> overrides, Vec2 position);
    void spawnChildren(Registry& registry, Entity parent, std::span<const PrefabChild> children, Vec2 position);
    void place(Registry& registry, Entity e, Vec2 position);

public:
    PrefabLibrary(const TypeRegistry& types, AssetManager& assets);
    PrefabLibrary(const PrefabLibrary&) = delete;
    PrefabLibrary& operator=(const PrefabLibrary&) = delete;

    Result<const Prefab*> get(std::string_view path);
    const Prefab* findByKey(uint32_t key) const;
    Entity instantiate(Registry& registry, const Prefab& prefab, Vec2 position,
                       std::span<const ComponentData> overrides = {});
    void instantiateInto(Registry& registry, Entity e, const Prefab& prefab, Vec2 position,
                         std::span<const ComponentData> overrides = {});
    void clear();
};

ENGINE_API uint32_t prefabKey(std::string_view path);

}

#include "engine/scene/Prefab.inl"
