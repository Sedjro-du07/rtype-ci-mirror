/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TypeInfo
*/

#pragma once

#include "engine/core/Error.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/IStorage.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace engine {

class AssetManager;
class YamlNode;
class YamlWriter;
struct LoadContext;
struct SaveContext;
class ByteWriter;
class ByteReader;

enum class FieldKind : uint8_t {
    Bool,
    Int8,
    UInt8,
    Int16,
    UInt16,
    Int32,
    UInt32,
    Int64,
    Float,
    Double,
    String,
    Vec2,
    Rect,
    Color,
    Entity,
    Texture,
    Sound,
    Music,
    Font,
    Script,
    Enum,
    List,
    Struct,
    Custom
};

struct EnumInfo {
    std::vector<std::pair<std::string, int64_t>> values;
    uint8_t size = sizeof(int32_t);
    bool isSigned = true;
};

struct ListOps {
    size_t (*size)(const void* list) = nullptr;
    void (*resize)(void* list, size_t count) = nullptr;
    void* (*at)(void* list, size_t index) = nullptr;
    const void* (*constAt)(const void* list, size_t index) = nullptr;
};

struct FieldInfo {
    std::string name;
    FieldKind kind = FieldKind::Custom;
    size_t offset = 0;
    const EnumInfo* enumInfo = nullptr;
    FieldKind element{};
    uint64_t elementType = 0;
    const ListOps* list = nullptr;
    uint32_t maxLength = 0;
    float quantMin = 0, quantMax = 0;
    std::string tooltip;
};

enum TypeFlags : uint32_t {
    None = 0,
    Replicated = 1 << 0,
    EditorHidden = 1 << 1,
    Tag = 1 << 2,
    Transient = 1 << 3,
};

enum class Transport : uint8_t { Unreliable, Reliable, Stream };

struct WireContext {
    std::function<uint32_t(Entity)> networkIdOf;
    std::function<Entity(uint32_t)> entityOf;
    AssetManager* assets = nullptr;
};

struct TypeInfo {
    ComponentId id = 0;
    uint64_t key = 0;
    std::string name;
    TypeCategory category = TypeCategory::Component;
    size_t size = 0, align = 0;
    uint32_t flags = None;
    std::vector<FieldInfo> fields;
    void (*construct)(void* dst) = nullptr;
    void (*destroy)(void* obj) noexcept = nullptr;
    void (*copy)(void* dst, const void* src) = nullptr;
    std::unique_ptr<IStorage> (*makeStorage)(ComponentId) = nullptr;
    Result<void> (*fromYaml)(void* obj, const YamlNode&, LoadContext&) = nullptr;
    void (*toYaml)(const void* obj, YamlWriter&, SaveContext&) = nullptr;
    void (*encode)(const void* obj, ByteWriter&, const WireContext&) = nullptr;
    Result<void> (*decode)(void* obj, ByteReader&, const WireContext&) = nullptr;
    Transport transport = Transport::Reliable;
};

}
