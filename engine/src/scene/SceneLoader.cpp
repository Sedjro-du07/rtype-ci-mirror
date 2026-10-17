/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SceneLoader
*/

#include "engine/scene/SceneLoader.hpp"

#include "engine/assets/AssetManager.hpp"
#include "engine/components/Hierarchy.hpp"
#include "engine/reflection/TypeRegistry.hpp"
#include "engine/reflection/Yaml.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
#include <fstream>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace engine {

namespace {

struct Link {
    const TypeInfo* type;
    YamlNode node;
};

struct Pending {
    Entity entity;
    std::vector<Link> links;
};

bool refersToEntities(const TypeRegistry& types, const TypeInfo& type, int depth = 0) {
    if (type.fromYaml) return true;
    if (depth > 8) return false;
    for (const FieldInfo& field : type.fields) {
        const FieldKind kind = field.kind == FieldKind::List ? field.element : field.kind;
        if (kind == FieldKind::Entity) return true;
        if (kind == FieldKind::Struct)
            if (const TypeInfo* inner = types.findByKey(field.elementType);
                inner && refersToEntities(types, *inner, depth + 1))
                return true;
    }
    return false;
}

Vec2 positionIn(const TypeRegistry& types, std::span<const ComponentData> components) {
    const TypeInfo* position = types.find("Position");
    if (!position) return {};
    for (const ComponentData& data : components) {
        if (data.type().id != position->id) continue;
        Vec2 result;
        for (const FieldInfo& field : position->fields) {
            if (field.kind != FieldKind::Float) continue;
            const auto* value =
                reinterpret_cast<const float*>(static_cast<const std::byte*>(data.value()) + field.offset);
            if (field.name == "x") result.x = *value;
            if (field.name == "y") result.y = *value;
        }
        return result;
    }
    return {};
}

void markDescendants(Scene& scene, Entity e) {
    for (Entity child : scene.registry().children(e)) {
        scene.markGenerated(child);
        markDescendants(scene, child);
    }
}

std::string writeComponent(const TypeInfo& type, const void* value, SaveContext& ctx) {
    YamlWriter out;
    writeYaml(type, value, out, ctx);
    return out.str();
}

}

SceneLoader::SceneLoader(const TypeRegistry& types, AssetManager& assets, PrefabLibrary& prefabs)
    : _types(&types), _assets(&assets), _prefabs(&prefabs) {}

Result<std::unique_ptr<Scene>> SceneLoader::load(const std::filesystem::path& file) {
    const std::string name = file.generic_string();
    const std::filesystem::path resolved = this->_assets->resolve(name);
    std::error_code ec;
    if (resolved.empty() || !std::filesystem::is_regular_file(resolved, ec))
        return fail(Errc::FileNotFound, std::format("unknown scene '{}'", name));
    auto root = YamlNode::load(resolved, name);
    if (!root) return unexpected(root.error());
    if (!root->isMap()) return root->error(Errc::ParseError, "a scene is a map with 'name' and 'entities'");

    std::string title = file.stem().string();
    YamlNode entities;
    for (const auto& [key, value] : root->entries()) {
        if (key == "name") {
            if (!value.isScalar()) return value.error(Errc::ParseError, "'name' expects a string");
            title = value.scalar();
        } else if (key == "entities") {
            if (!value.isNull() && !value.isSequence())
                return value.error(Errc::ParseError, "'entities' expects a list of entities");
            entities = value;
        } else {
            return value.error(Errc::ParseError,
                               std::format("unknown scene key '{}' (expected 'name' or 'entities')", key));
        }
    }

    auto scene = std::make_unique<Scene>(*this->_types, title, file);
    Registry& registry = scene->registry();
    const TypeInfo* uuidType = this->_types->find("Uuid");
    if (uuidType && uuidType->category != TypeCategory::Component) uuidType = nullptr;
    std::unordered_map<Uuid, Entity> byUuid;
    std::vector<Pending> pending;
    LoadContext firstPass{*this->_types, this->_assets, {}};

    for (size_t i = 0; i < entities.size(); ++i) {
        const YamlNode entry = entities[i];
        if (!entry.isMap())
            return entry.error(Errc::ParseError, std::format("an entity is a map, got {}", entry.describe()));
        const Prefab* prefab = nullptr;
        std::optional<Uuid> uuid;
        std::vector<ComponentData> components;
        Pending links{Entity::null(), {}};
        for (const auto& [key, value] : entry.entries()) {
            if (key == "uuid") {
                uuid = value.isScalar() ? parseUuid(value.scalar()) : std::nullopt;
                if (!uuid)
                    return value.error(Errc::ParseError,
                                       std::format("'uuid' expects hexadecimal digits, got {}", value.describe()));
                if (byUuid.contains(*uuid))
                    return value.error(Errc::DuplicateName, std::format("uuid '{}' is used twice", formatUuid(*uuid)));
                continue;
            }
            if (key == "prefab") {
                if (!value.isScalar()) return value.error(Errc::ParseError, "'prefab' expects a file path");
                auto found = this->_prefabs->get(value.scalar());
                if (!found) {
                    if (found.error().file.empty()) return value.error(found.error().code, found.error().message);
                    return unexpected(found.error());
                }
                prefab = *found;
                continue;
            }
            if (key == "children")
                return value.error(Errc::ParseError, "children are declared in prefabs, not in scenes");
            const TypeInfo* type = this->_types->find(key);
            if (!type || type->category != TypeCategory::Component) {
                const std::string close = this->_types->closestName(key);
                return value.error(
                    Errc::UnknownComponent,
                    std::format("unknown component '{}'{}", key,
                                close.empty() ? std::string{} : std::format(" (did you mean '{}'?)", close)));
            }
            auto data = ComponentData::parse(*type, value, firstPass);
            if (!data) return unexpected(data.error());
            if (refersToEntities(*this->_types, *type)) links.links.push_back(Link{type, value});
            components.push_back(std::move(*data));
        }

        const Entity e = registry.create();
        if (prefab) {
            this->_prefabs->instantiateInto(registry, e, *prefab, positionIn(*this->_types, components), components);
            scene->setPrefab(e, prefab->path);
            markDescendants(*scene, e);
        } else {
            for (const ComponentData& data : components)
                registry.emplaceRaw(e, data.type().id, data.value());
        }
        if (uuid) {
            byUuid.emplace(*uuid, e);
            scene->setUuid(e, *uuid);
            if (uuidType) registry.emplaceRaw(e, uuidType->id, &*uuid);
        }
        links.entity = e;
        if (!links.links.empty()) pending.push_back(std::move(links));
    }

    std::optional<Uuid> missing;
    LoadContext secondPass{*this->_types, this->_assets, [&](const Uuid& uuid) {
                               const auto it = byUuid.find(uuid);
                               if (it != byUuid.end()) return it->second;
                               missing = uuid;
                               return Entity::null();
                           }};
    const TypeInfo* parentType = this->_types->find("Parent");
    for (const Pending& entity : pending) {
        for (const Link& link : entity.links) {
            missing.reset();
            void* live = registry.getRaw(entity.entity, link.type->id);
            if (!live) continue;
            if (auto ok = readYaml(*link.type, live, link.node, secondPass); !ok) return unexpected(ok.error());
            if (missing)
                return link.node.error(Errc::ParseError,
                                       std::format("{} refers to uuid '{}', which no entity of this scene has",
                                                   link.type->name, formatUuid(*missing)));
        }
        if (!parentType) continue;
        const auto* parent = static_cast<const Parent*>(registry.getRaw(entity.entity, parentType->id));
        if (!parent || parent->value.isNull()) continue;
        const Parent copy = *parent;
        for (Entity up = copy.value; !up.isNull(); up = registry.parent(up))
            if (up == entity.entity)
                return fail(Errc::ParseError, "parents form a loop: an entity cannot be its own ancestor", name);
        registry.removeRaw(entity.entity, parentType->id);
        registry.setParent(entity.entity, copy.value, copy.offset);
    }
    return scene;
}

Result<void> SceneLoader::save(const Scene& scene, const std::filesystem::path& file) const {
    const Registry& registry = scene.registry();
    std::unordered_map<Entity, Uuid> fresh;
    const auto uuidOf = [&](Entity e) -> std::optional<Uuid> {
        if (!registry.alive(e) || scene.generated(e)) return std::nullopt;
        if (auto known = scene.uuidOf(e)) return known;
        auto [it, inserted] = fresh.try_emplace(e, Uuid{});
        if (inserted) it->second = Uuid::generate();
        return it->second;
    };
    SaveContext ctx{*this->_types, this->_assets, uuidOf};

    YamlWriter out;
    out.beginMap();
    out.key("name");
    out.value(std::string_view(scene.name()));
    out.key("entities");
    out.beginSequence();
    for (Entity e : registry.entities()) {
        if (scene.generated(e)) continue;
        const std::string_view prefabPath = scene.prefabOf(e);
        const Prefab* prefab = nullptr;
        if (!prefabPath.empty())
            if (auto found = this->_prefabs->get(prefabPath)) prefab = *found;
        out.beginMap();
        out.key("uuid");
        out.value(formatUuid(*uuidOf(e)));
        if (prefab) {
            out.key("prefab");
            out.value(prefabPath);
        }
        for (ComponentId id : registry.componentsOf(e)) {
            const TypeInfo& type = this->_types->info(id);
            if ((type.flags & Transient) || type.name == "Uuid" || type.name == "Children") continue;
            const void* value = registry.getRaw(e, id);
            const std::string written = writeComponent(type, value, ctx);
            if (prefab) {
                const auto base =
                    std::ranges::find(prefab->components, id, [](const ComponentData& c) { return c.type().id; });
                if (base != prefab->components.end() && writeComponent(type, base->value(), ctx) == written) continue;
            }
            out.key(type.name);
            writeYaml(type, value, out, ctx);
        }
        out.endMap();
    }
    out.endSequence();
    out.endMap();

    std::ofstream stream(file, std::ios::binary | std::ios::trunc);
    if (!stream) return fail(Errc::FileNotFound, "cannot write the scene file", file.generic_string());
    stream << out.str() << '\n';
    if (!stream) return fail(Errc::FileNotFound, "cannot write the scene file", file.generic_string());
    return {};
}

}
