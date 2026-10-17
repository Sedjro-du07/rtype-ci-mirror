/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** YamlCodec
*/

#include "engine/assets/AssetManager.hpp"
#include "engine/core/math/Color.hpp"
#include "engine/core/math/Rect.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/reflection/TypeRegistry.hpp"
#include "engine/reflection/Yaml.hpp"
#include "engine/render/Handles.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace engine {

namespace {

struct Slot {
    const FieldInfo& field;
    FieldKind kind;
    std::string_view owner;
};

size_t editDistance(std::string_view a, std::string_view b) {
    std::vector<size_t> row(b.size() + 1);
    for (size_t j = 0; j <= b.size(); ++j)
        row[j] = j;
    for (size_t i = 1; i <= a.size(); ++i) {
        size_t diagonal = row[0];
        row[0] = i;
        for (size_t j = 1; j <= b.size(); ++j) {
            const size_t above = row[j];
            const bool same = std::tolower(static_cast<unsigned char>(a[i - 1])) ==
                              std::tolower(static_cast<unsigned char>(b[j - 1]));
            row[j] = std::min({row[j] + 1, row[j - 1] + 1, diagonal + (same ? 0 : 1)});
            diagonal = above;
        }
    }
    return row[b.size()];
}

std::string suggestion(std::string_view unknown, const std::vector<std::string_view>& names) {
    const size_t limit = std::max<size_t>(2, unknown.size() / 3);
    std::string_view best;
    size_t bestDistance = limit + 1;
    for (std::string_view name : names) {
        const size_t distance = editDistance(unknown, name);
        if (distance < bestDistance) {
            bestDistance = distance;
            best = name;
        }
    }
    return best.empty() ? std::string{} : std::format(" (did you mean '{}'?)", best);
}

unexpected<Error> mismatch(const YamlNode& node, const Slot& slot, std::string_view expected) {
    return node.error(Errc::ParseError, std::format("field '{}' of {} expects {}, got {}", slot.field.name, slot.owner,
                                                    expected, node.describe()));
}

template <class T>
Result<void> readInteger(void* target, const YamlNode& node, const Slot& slot) {
    constexpr auto low = std::numeric_limits<T>::min();
    constexpr auto high = std::numeric_limits<T>::max();
    const std::string expected = std::format("an integer from {} to {}", low, high);
    T value{};
    if constexpr (std::is_signed_v<T>) {
        const auto number = node.asInt();
        if (!number || *number < low || *number > high) return mismatch(node, slot, expected);
        value = static_cast<T>(*number);
    } else {
        const auto number = node.asUInt();
        if (!number || *number > high) return mismatch(node, slot, expected);
        value = static_cast<T>(*number);
    }
    std::memcpy(target, &value, sizeof(T));
    return {};
}

template <size_t N>
Result<void> readFloats(std::array<float, N>& out, const std::array<std::string_view, N>& names, const YamlNode& node,
                        const Slot& slot) {
    const std::string expected = std::format("{{{}}} or a list of {} numbers", std::string_view(names[0]), N);
    if (node.isSequence()) {
        if (node.size() != N) return mismatch(node, slot, expected);
        for (size_t i = 0; i < N; ++i) {
            const auto number = node[i].asDouble();
            if (!number) return mismatch(node[i], slot, "a number");
            out[i] = static_cast<float>(*number);
        }
        return {};
    }
    if (!node.isMap()) return mismatch(node, slot, expected);
    for (const auto& [key, value] : node.entries()) {
        const auto it = std::ranges::find(names, key);
        if (it == names.end()) {
            const std::vector<std::string_view> known(names.begin(), names.end());
            return value.error(Errc::ParseError, std::format("unknown key '{}' in field '{}' of {}{}", key,
                                                             slot.field.name, slot.owner, suggestion(key, known)));
        }
        const auto number = value.asDouble();
        if (!number) return mismatch(value, slot, "a number");
        out[static_cast<size_t>(it - names.begin())] = static_cast<float>(*number);
    }
    return {};
}

std::optional<uint8_t> hexByte(std::string_view text) {
    uint8_t value = 0;
    for (char c : text) {
        value = static_cast<uint8_t>(value << 4);
        if (c >= '0' && c <= '9')
            value = static_cast<uint8_t>(value | (c - '0'));
        else if (c >= 'a' && c <= 'f')
            value = static_cast<uint8_t>(value | (c - 'a' + 10));
        else if (c >= 'A' && c <= 'F')
            value = static_cast<uint8_t>(value | (c - 'A' + 10));
        else
            return std::nullopt;
    }
    return value;
}

Result<void> readColor(Color& color, const YamlNode& node, const Slot& slot) {
    constexpr std::string_view expected = "\"#RRGGBB\", \"#RRGGBBAA\", {r, g, b, a} or a list of 3 or 4 integers";
    std::array<uint8_t*, 4> channels{&color.r, &color.g, &color.b, &color.a};
    if (node.isScalar()) {
        const std::string text = node.scalar();
        if (!text.starts_with('#') || (text.size() != 7 && text.size() != 9)) return mismatch(node, slot, expected);
        color.a = 255;
        for (size_t i = 0; i * 2 + 1 < text.size(); ++i) {
            const auto byte = hexByte(std::string_view(text).substr(1 + i * 2, 2));
            if (!byte) return mismatch(node, slot, expected);
            *channels[i] = *byte;
        }
        return {};
    }
    if (node.isSequence()) {
        if (node.size() != 3 && node.size() != 4) return mismatch(node, slot, expected);
        color.a = 255;
        for (size_t i = 0; i < node.size(); ++i) {
            const auto number = node[i].asUInt();
            if (!number || *number > 255) return mismatch(node[i], slot, "an integer from 0 to 255");
            *channels[i] = static_cast<uint8_t>(*number);
        }
        return {};
    }
    if (!node.isMap()) return mismatch(node, slot, expected);
    constexpr std::array<std::string_view, 4> names{"r", "g", "b", "a"};
    for (const auto& [key, value] : node.entries()) {
        const auto it = std::ranges::find(names, key);
        if (it == names.end())
            return value.error(Errc::ParseError,
                               std::format("unknown key '{}' in field '{}' of {}", key, slot.field.name, slot.owner));
        const auto number = value.asUInt();
        if (!number || *number > 255) return mismatch(value, slot, "an integer from 0 to 255");
        *channels[static_cast<size_t>(it - names.begin())] = static_cast<uint8_t>(*number);
    }
    return {};
}

void storeEnum(void* target, const EnumInfo& info, int64_t value) {
    switch (info.size) {
    case 1: {
        const auto v = static_cast<uint8_t>(value);
        std::memcpy(target, &v, 1);
        break;
    }
    case 2: {
        const auto v = static_cast<uint16_t>(value);
        std::memcpy(target, &v, 2);
        break;
    }
    case 4: {
        const auto v = static_cast<uint32_t>(value);
        std::memcpy(target, &v, 4);
        break;
    }
    default:
        std::memcpy(target, &value, 8);
    }
}

int64_t loadEnum(const void* source, const EnumInfo& info) {
    switch (info.size) {
    case 1: {
        uint8_t v = 0;
        std::memcpy(&v, source, 1);
        return info.isSigned ? static_cast<int8_t>(v) : v;
    }
    case 2: {
        uint16_t v = 0;
        std::memcpy(&v, source, 2);
        return info.isSigned ? static_cast<int16_t>(v) : v;
    }
    case 4: {
        uint32_t v = 0;
        std::memcpy(&v, source, 4);
        return info.isSigned ? static_cast<int32_t>(v) : static_cast<int64_t>(v);
    }
    default: {
        int64_t v = 0;
        std::memcpy(&v, source, 8);
        return v;
    }
    }
}

template <class H>
Result<void> readHandle(void* target, const YamlNode& node, const Slot& slot, LoadContext& ctx) {
    H handle{};
    if (!node.isNull()) {
        if (!node.isScalar()) return mismatch(node, slot, "a file path");
        const std::string path = node.scalar();
        if (!path.empty()) {
            if (!ctx.assets)
                return node.error(Errc::AssetLoadFailed,
                                  std::format("field '{}' of {} needs the asset manager", slot.field.name, slot.owner));
            if constexpr (std::is_same_v<H, TextureHandle>)
                handle = ctx.assets->texture(path);
            else if constexpr (std::is_same_v<H, FontHandle>)
                handle = ctx.assets->font(path);
            else if constexpr (std::is_same_v<H, SoundHandle>)
                handle = ctx.assets->sound(path);
            else if constexpr (std::is_same_v<H, MusicHandle>)
                handle = ctx.assets->music(path);
            else
                handle = ctx.assets->script(path);
        }
    }
    std::memcpy(target, &handle, sizeof(H));
    return {};
}

template <class H>
void writeHandle(const void* source, YamlWriter& out, SaveContext& ctx) {
    H handle{};
    std::memcpy(&handle, source, sizeof(H));
    const std::string_view path = handle.valid() && ctx.assets ? ctx.assets->pathOf(handle) : std::string_view{};
    if (path.empty())
        out.null();
    else
        out.value(path);
}

Result<void> readValue(void* target, const YamlNode& node, const Slot& slot, LoadContext& ctx);

Result<void> readList(void* target, const YamlNode& node, const Slot& slot, LoadContext& ctx) {
    const ListOps& ops = *slot.field.list;
    if (node.isNull()) {
        ops.resize(target, 0);
        return {};
    }
    if (!node.isSequence()) return mismatch(node, slot, "a list");
    if (slot.field.maxLength != 0 && node.size() > slot.field.maxLength)
        return node.error(Errc::ParseError,
                          std::format("field '{}' of {} holds at most {} elements, got {}", slot.field.name, slot.owner,
                                      slot.field.maxLength, node.size()));
    ops.resize(target, node.size());
    const Slot element{slot.field, slot.field.element, slot.owner};
    for (size_t i = 0; i < node.size(); ++i)
        if (auto ok = readValue(ops.at(target, i), node[i], element, ctx); !ok) return ok;
    return {};
}

Result<void> readValue(void* target, const YamlNode& node, const Slot& slot, LoadContext& ctx) {
    switch (slot.kind) {
    case FieldKind::Bool: {
        const auto flag = node.asBool();
        if (!flag) return mismatch(node, slot, "true or false");
        *static_cast<bool*>(target) = *flag;
        return {};
    }
    case FieldKind::Int8:
        return readInteger<int8_t>(target, node, slot);
    case FieldKind::UInt8:
        return readInteger<uint8_t>(target, node, slot);
    case FieldKind::Int16:
        return readInteger<int16_t>(target, node, slot);
    case FieldKind::UInt16:
        return readInteger<uint16_t>(target, node, slot);
    case FieldKind::Int32:
        return readInteger<int32_t>(target, node, slot);
    case FieldKind::UInt32:
        return readInteger<uint32_t>(target, node, slot);
    case FieldKind::Int64:
        return readInteger<int64_t>(target, node, slot);
    case FieldKind::Float: {
        const auto number = node.asDouble();
        if (!number) return mismatch(node, slot, "a number");
        *static_cast<float*>(target) = static_cast<float>(*number);
        return {};
    }
    case FieldKind::Double: {
        const auto number = node.asDouble();
        if (!number) return mismatch(node, slot, "a number");
        *static_cast<double*>(target) = *number;
        return {};
    }
    case FieldKind::String: {
        if (node.isNull()) {
            static_cast<std::string*>(target)->clear();
            return {};
        }
        if (!node.isScalar()) return mismatch(node, slot, "a string");
        std::string text = node.scalar();
        if (slot.field.maxLength != 0 && text.size() > slot.field.maxLength)
            return node.error(Errc::ParseError,
                              std::format("field '{}' of {} holds at most {} bytes, got {}", slot.field.name,
                                          slot.owner, slot.field.maxLength, text.size()));
        *static_cast<std::string*>(target) = std::move(text);
        return {};
    }
    case FieldKind::Vec2: {
        auto& v = *static_cast<Vec2*>(target);
        std::array<float, 2> values{v.x, v.y};
        if (auto ok = readFloats<2>(values, {"x", "y"}, node, slot); !ok) return ok;
        v = Vec2{values[0], values[1]};
        return {};
    }
    case FieldKind::Rect: {
        auto& r = *static_cast<Rect*>(target);
        std::array<float, 4> values{r.x, r.y, r.w, r.h};
        if (auto ok = readFloats<4>(values, {"x", "y", "w", "h"}, node, slot); !ok) return ok;
        r = Rect{values[0], values[1], values[2], values[3]};
        return {};
    }
    case FieldKind::Color:
        return readColor(*static_cast<Color*>(target), node, slot);
    case FieldKind::Entity: {
        auto& entity = *static_cast<Entity*>(target);
        if (node.isNull() || node.scalar().empty()) {
            entity = Entity::null();
            return {};
        }
        const auto uuid = node.isScalar() ? parseUuid(node.scalar()) : std::nullopt;
        if (!uuid) return mismatch(node, slot, "an entity uuid (hexadecimal)");
        entity = ctx.entityOf ? ctx.entityOf(*uuid) : Entity::null();
        return {};
    }
    case FieldKind::Texture:
        return readHandle<TextureHandle>(target, node, slot, ctx);
    case FieldKind::Sound:
        return readHandle<SoundHandle>(target, node, slot, ctx);
    case FieldKind::Music:
        return readHandle<MusicHandle>(target, node, slot, ctx);
    case FieldKind::Font:
        return readHandle<FontHandle>(target, node, slot, ctx);
    case FieldKind::Script:
        return readHandle<ScriptHandle>(target, node, slot, ctx);
    case FieldKind::Enum: {
        const EnumInfo& info = *slot.field.enumInfo;
        std::vector<std::string_view> names;
        for (const auto& [name, value] : info.values)
            names.push_back(name);
        if (!node.isScalar()) return mismatch(node, slot, "a value name");
        const std::string text = node.scalar();
        for (const auto& [name, value] : info.values)
            if (name == text) {
                storeEnum(target, info, value);
                return {};
            }
        return node.error(Errc::ParseError, std::format("field '{}' of {} has no value '{}'{}", slot.field.name,
                                                        slot.owner, text, suggestion(text, names)));
    }
    case FieldKind::List:
        return readList(target, node, slot, ctx);
    case FieldKind::Struct: {
        const TypeInfo* type = ctx.types.findByKey(slot.field.elementType);
        if (!type)
            return node.error(Errc::UnknownComponent,
                              std::format("field '{}' of {} uses an unregistered struct", slot.field.name, slot.owner));
        return readYaml(*type, target, node, ctx);
    }
    case FieldKind::Custom:
        break;
    }
    return node.error(Errc::ParseError, std::format("field '{}' of {} has no YAML form: give the type a custom reader",
                                                    slot.field.name, slot.owner));
}

void writeFloats(YamlWriter& out, std::span<const std::string_view> names, std::span<const float> values) {
    out.beginMap(true);
    for (size_t i = 0; i < names.size(); ++i) {
        out.key(names[i]);
        out.value(values[i]);
    }
    out.endMap();
}

void writeValue(const void* source, YamlWriter& out, const FieldInfo& field, FieldKind kind, SaveContext& ctx) {
    switch (kind) {
    case FieldKind::Bool:
        out.value(*static_cast<const bool*>(source));
        return;
    case FieldKind::Int8:
        out.value(static_cast<int64_t>(*static_cast<const int8_t*>(source)));
        return;
    case FieldKind::UInt8:
        out.value(static_cast<uint64_t>(*static_cast<const uint8_t*>(source)));
        return;
    case FieldKind::Int16:
        out.value(static_cast<int64_t>(*static_cast<const int16_t*>(source)));
        return;
    case FieldKind::UInt16:
        out.value(static_cast<uint64_t>(*static_cast<const uint16_t*>(source)));
        return;
    case FieldKind::Int32:
        out.value(static_cast<int64_t>(*static_cast<const int32_t*>(source)));
        return;
    case FieldKind::UInt32:
        out.value(static_cast<uint64_t>(*static_cast<const uint32_t*>(source)));
        return;
    case FieldKind::Int64:
        out.value(*static_cast<const int64_t*>(source));
        return;
    case FieldKind::Float:
        out.value(*static_cast<const float*>(source));
        return;
    case FieldKind::Double:
        out.value(*static_cast<const double*>(source));
        return;
    case FieldKind::String:
        out.value(std::string_view(*static_cast<const std::string*>(source)));
        return;
    case FieldKind::Vec2: {
        const auto& v = *static_cast<const Vec2*>(source);
        constexpr std::array<std::string_view, 2> names{"x", "y"};
        const std::array<float, 2> values{v.x, v.y};
        writeFloats(out, names, values);
        return;
    }
    case FieldKind::Rect: {
        const auto& r = *static_cast<const Rect*>(source);
        constexpr std::array<std::string_view, 4> names{"x", "y", "w", "h"};
        const std::array<float, 4> values{r.x, r.y, r.w, r.h};
        writeFloats(out, names, values);
        return;
    }
    case FieldKind::Color: {
        const auto& c = *static_cast<const Color*>(source);
        if (c.a == 255)
            out.value(std::format("#{:02x}{:02x}{:02x}", c.r, c.g, c.b));
        else
            out.value(std::format("#{:02x}{:02x}{:02x}{:02x}", c.r, c.g, c.b, c.a));
        return;
    }
    case FieldKind::Entity: {
        const Entity entity = *static_cast<const Entity*>(source);
        const auto uuid = !entity.isNull() && ctx.uuidOf ? ctx.uuidOf(entity) : std::nullopt;
        if (uuid)
            out.value(formatUuid(*uuid));
        else
            out.null();
        return;
    }
    case FieldKind::Texture:
        writeHandle<TextureHandle>(source, out, ctx);
        return;
    case FieldKind::Sound:
        writeHandle<SoundHandle>(source, out, ctx);
        return;
    case FieldKind::Music:
        writeHandle<MusicHandle>(source, out, ctx);
        return;
    case FieldKind::Font:
        writeHandle<FontHandle>(source, out, ctx);
        return;
    case FieldKind::Script:
        writeHandle<ScriptHandle>(source, out, ctx);
        return;
    case FieldKind::Enum: {
        const EnumInfo& info = *field.enumInfo;
        const int64_t value = loadEnum(source, info);
        for (const auto& [name, number] : info.values)
            if (number == value) {
                out.value(std::string_view(name));
                return;
            }
        out.value(value);
        return;
    }
    case FieldKind::List: {
        const ListOps& ops = *field.list;
        out.beginSequence(true);
        for (size_t i = 0; i < ops.size(source); ++i)
            writeValue(ops.constAt(source, i), out, field, field.element, ctx);
        out.endSequence();
        return;
    }
    case FieldKind::Struct: {
        const TypeInfo* type = ctx.types.findByKey(field.elementType);
        if (type)
            writeYaml(*type, source, out, ctx);
        else
            out.null();
        return;
    }
    case FieldKind::Custom:
        out.null();
        return;
    }
}

}

std::string formatUuid(const Uuid& uuid) {
    if (uuid.hi == 0) return std::format("{:016x}", uuid.lo);
    return std::format("{:016x}{:016x}", uuid.hi, uuid.lo);
}

std::optional<Uuid> parseUuid(std::string_view text) {
    Uuid uuid;
    size_t digits = 0;
    for (char c : text) {
        if (c == '-') continue;
        const auto nibble = hexByte(std::string_view(&c, 1));
        if (!nibble || ++digits > 32) return std::nullopt;
        uuid.hi = (uuid.hi << 4) | (uuid.lo >> 60);
        uuid.lo = (uuid.lo << 4) | *nibble;
    }
    if (digits == 0) return std::nullopt;
    return uuid;
}

Result<void> readYaml(const TypeInfo& type, void* object, const YamlNode& node, LoadContext& ctx) {
    if (type.fromYaml) return type.fromYaml(object, node, ctx);
    if (node.isNull()) return {};
    if (!node.isMap())
        return node.error(Errc::ParseError,
                          std::format("{} expects a map of fields, got {}", type.name, node.describe()));
    for (const auto& [key, value] : node.entries()) {
        const auto field = std::ranges::find(type.fields, key, &FieldInfo::name);
        if (field == type.fields.end()) {
            std::vector<std::string_view> names;
            for (const FieldInfo& known : type.fields)
                names.push_back(known.name);
            return value.error(Errc::ParseError,
                               std::format("unknown field '{}' in {}{}", key, type.name, suggestion(key, names)));
        }
        const Slot slot{*field, field->kind, type.name};
        if (auto ok = readValue(static_cast<std::byte*>(object) + field->offset, value, slot, ctx); !ok) return ok;
    }
    return {};
}

void writeYaml(const TypeInfo& type, const void* object, YamlWriter& out, SaveContext& ctx) {
    if (type.toYaml) {
        type.toYaml(object, out, ctx);
        return;
    }
    out.beginMap(true);
    for (const FieldInfo& field : type.fields) {
        out.key(field.name);
        writeValue(static_cast<const std::byte*>(object) + field.offset, out, field, field.kind, ctx);
    }
    out.endMap();
}

}
