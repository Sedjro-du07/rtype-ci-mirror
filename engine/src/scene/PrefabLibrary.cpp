/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** PrefabLibrary
*/

#include "engine/assets/AssetManager.hpp"
#include "engine/components/Hierarchy.hpp"
#include "engine/components/Position.hpp"
#include "engine/core/Hash.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/net/NetComponents.hpp"
#include "engine/reflection/TypeRegistry.hpp"
#include "engine/scene/Prefab.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <format>
#include <ranges>
#include <utility>

namespace engine {

namespace {

void put(Registry& registry, Entity e, const ComponentData& data) {
    registry.removeRaw(e, data.type().id);
    registry.emplaceRaw(e, data.type().id, data.value());
}

}

uint32_t prefabKey(std::string_view path) {
    const uint64_t hash = fnv1a64(path);
    return static_cast<uint32_t>(hash ^ (hash >> 32));
}

PrefabLibrary::PrefabLibrary(const TypeRegistry& types, AssetManager& assets) : _types(&types), _assets(&assets) {}

Result<void> PrefabLibrary::parseBody(const YamlNode& node, bool child, std::vector<ComponentData>& components,
                                      std::vector<PrefabChild>& children, PrefabChild* self) {
    if (node.isNull()) return {};
    if (!node.isMap())
        return node.error(Errc::ParseError, std::format("an entity is a map of components, got {}", node.describe()));
    LoadContext ctx{*this->_types, this->_assets, {}};
    for (const auto& [key, value] : node.entries()) {
        if (key == "children") {
            if (!value.isSequence()) return value.error(Errc::ParseError, "'children' expects a list of entities");
            if (!this->_types->find("Parent"))
                return value.error(Errc::UnknownComponent, "children need the engine component 'Parent' registered");
            for (size_t i = 0; i < value.size(); ++i) {
                auto parsed = this->parseChild(value[i]);
                if (!parsed) return unexpected(parsed.error());
                children.push_back(std::move(*parsed));
            }
            continue;
        }
        if (key == "prefab") {
            if (!child) return value.error(Errc::ParseError, "a prefab cannot name another prefab: use 'children'");
            if (!value.isScalar()) return value.error(Errc::ParseError, "'prefab' expects a file path");
            self->prefab = value.scalar();
            auto resolved = this->get(self->prefab);
            if (!resolved) return unexpected(resolved.error());
            self->resolved = *resolved;
            continue;
        }
        const TypeInfo* type = this->_types->find(key);
        if (!type || type->category != TypeCategory::Component) {
            const std::string close = this->_types->closestName(key);
            return value.error(
                Errc::UnknownComponent,
                std::format("unknown component '{}'{}", key,
                            close.empty() ? std::string{} : std::format(" (did you mean '{}'?)", close)));
        }
        auto data = ComponentData::parse(*type, value, ctx);
        if (!data) return unexpected(data.error());
        if (child && type->name == "Parent") {
            self->offset = static_cast<const Parent*>(data->value())->offset;
            continue;
        }
        components.push_back(std::move(*data));
    }
    return {};
}

Result<PrefabChild> PrefabLibrary::parseChild(const YamlNode& node) {
    PrefabChild child;
    if (auto ok = this->parseBody(node, true, child.components, child.children, &child); !ok)
        return unexpected(ok.error());
    return child;
}

Result<const Prefab*> PrefabLibrary::get(std::string_view path) {
    const std::string name(path);
    if (const auto it = this->_cache.find(name); it != this->_cache.end()) return it->second.get();
    if (std::ranges::contains(this->_loading, name))
        return fail(Errc::UnknownPrefab, std::format("prefab '{}' includes itself", name), name);
    const std::filesystem::path file = this->_assets->resolve(path);
    std::error_code ec;
    if (file.empty() || !std::filesystem::is_regular_file(file, ec))
        return fail(Errc::UnknownPrefab, std::format("unknown prefab '{}'", name));
    auto node = YamlNode::load(file, name);
    if (!node) return unexpected(node.error());

    auto prefab = std::make_unique<Prefab>();
    prefab->path = name;
    prefab->key = prefabKey(name);
    if (const auto other = this->_byKey.find(prefab->key); other != this->_byKey.end())
        return fail(Errc::DuplicateName,
                    std::format("prefabs '{}' and '{}' have the same key: rename one", other->second->path, name),
                    name);
    this->_loading.push_back(name);
    auto ok = this->parseBody(*node, false, prefab->components, prefab->children, nullptr);
    this->_loading.pop_back();
    if (!ok) return unexpected(ok.error());

    const Prefab* stored = prefab.get();
    this->_byKey.emplace(stored->key, stored);
    this->_cache.emplace(name, std::move(prefab));
    return stored;
}

const Prefab* PrefabLibrary::findByKey(uint32_t key) const {
    const auto it = this->_byKey.find(key);
    return it == this->_byKey.end() ? nullptr : it->second;
}

void PrefabLibrary::place(Registry& registry, Entity e, Vec2 position) {
    const TypeInfo* type = this->_types->findByKey(typeKey<Position>());
    if (!type) return;
    const Position value{position.x, position.y};
    if (auto* existing = static_cast<Position*>(registry.getRaw(e, type->id)))
        *existing = value;
    else
        registry.emplaceRaw(e, type->id, &value);
}

void PrefabLibrary::build(Registry& registry, Entity e, std::span<const ComponentData> base,
                          std::span<const ComponentData> overrides, Vec2 position) {
    LoadContext ctx{*this->_types, this->_assets, {}};
    for (const ComponentData& component : base) {
        const auto match =
            std::ranges::find(overrides, component.type().id, [](const ComponentData& o) { return o.type().id; });
        if (match == overrides.end()) {
            put(registry, e, component);
            continue;
        }
        if (!match->source().defined()) {
            put(registry, e, *match);
            continue;
        }
        ComponentData merged = component;
        (void)readYaml(merged.type(), merged.value(), match->source(), ctx);
        put(registry, e, merged);
    }
    for (const ComponentData& extra : overrides)
        if (std::ranges::none_of(base, [&](const ComponentData& c) { return c.type().id == extra.type().id; }))
            put(registry, e, extra);
    this->place(registry, e, position);
}

void PrefabLibrary::spawnChildren(Registry& registry, Entity parent, std::span<const PrefabChild> children,
                                  Vec2 position) {
    for (const PrefabChild& child : children) {
        const Entity e = registry.create();
        const Vec2 at{position.x + child.offset.x, position.y + child.offset.y};
        if (child.resolved) {
            this->build(registry, e, child.resolved->components, child.components, at);
            this->spawnChildren(registry, e, child.resolved->children, at);
        } else {
            this->build(registry, e, child.components, {}, at);
        }
        this->spawnChildren(registry, e, child.children, at);
        registry.setParent(e, parent, child.offset);
    }
}

Entity PrefabLibrary::instantiate(Registry& registry, const Prefab& prefab, Vec2 position,
                                  std::span<const ComponentData> overrides) {
    const Entity e = registry.create();
    this->instantiateInto(registry, e, prefab, position, overrides);
    return e;
}

void PrefabLibrary::instantiateInto(Registry& registry, Entity e, const Prefab& prefab, Vec2 position,
                                    std::span<const ComponentData> overrides) {
    this->build(registry, e, prefab.components, overrides, position);
    this->spawnChildren(registry, e, prefab.children, position);
    this->markNetworked(registry, e, prefab);
}

void PrefabLibrary::markNetworked(Registry& registry, Entity e, const Prefab& prefab) const {
    const TypeInfo* net = this->_types->findByKey(typeKey<NetPrefab>());
    if (!net) return;
    if (void* raw = registry.getRaw(e, net->id)) static_cast<NetPrefab*>(raw)->prefabKey = prefab.key;
}

Result<size_t> PrefabLibrary::preload(std::string_view folder) {
    const std::filesystem::path root = this->_assets->resolve(folder);
    std::error_code ec;
    if (root.empty() || !std::filesystem::is_directory(root, ec)) return size_t{0};
    size_t loaded = 0;
    for (const auto& file : std::filesystem::recursive_directory_iterator(root, ec)) {
        if (!file.is_regular_file() || file.path().extension() != ".yaml") continue;
        const std::string path =
            (std::filesystem::path(folder) / file.path().lexically_relative(root)).generic_string();
        if (auto prefab = this->get(path); !prefab) return unexpected(prefab.error());
        ++loaded;
    }
    return loaded;
}

void PrefabLibrary::clear() {
    this->_cache.clear();
    this->_byKey.clear();
}

}
