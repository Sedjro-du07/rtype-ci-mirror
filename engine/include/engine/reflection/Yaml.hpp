/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Yaml
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/components/Identity.hpp"
#include "engine/core/Error.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/reflection/TypeInfo.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace engine {

class AssetManager;
class TypeRegistry;

class ENGINE_API YamlNode {
private:
    struct Impl;
    std::shared_ptr<const Impl> _impl;

    explicit YamlNode(std::shared_ptr<const Impl> impl);

public:
    YamlNode();

    static Result<YamlNode> parse(std::string_view text, std::string file);
    static Result<YamlNode> load(const std::filesystem::path& path, std::string file);

    bool defined() const;
    bool isNull() const;
    bool isScalar() const;
    bool isSequence() const;
    bool isMap() const;

    size_t size() const;
    YamlNode operator[](size_t index) const;
    YamlNode get(std::string_view key) const;
    std::vector<std::pair<std::string, YamlNode>> entries() const;

    std::string scalar() const;
    std::optional<bool> asBool() const;
    std::optional<int64_t> asInt() const;
    std::optional<uint64_t> asUInt() const;
    std::optional<double> asDouble() const;

    std::string_view file() const;
    int line() const;
    std::string describe() const;
    unexpected<Error> error(Errc code, std::string message) const;
};

class ENGINE_API YamlWriter {
private:
    struct Impl;
    std::unique_ptr<Impl> _impl;

public:
    YamlWriter();
    ~YamlWriter();
    YamlWriter(const YamlWriter&) = delete;
    YamlWriter& operator=(const YamlWriter&) = delete;

    void beginMap(bool flow = false);
    void endMap();
    void beginSequence(bool flow = false);
    void endSequence();
    void key(std::string_view name);
    void value(std::string_view text);
    void value(const char* text);
    void value(bool flag);
    void value(int64_t number);
    void value(uint64_t number);
    void value(double number);
    void value(float number);
    void null();
    std::string str() const;
};

struct LoadContext {
    const TypeRegistry& types;
    AssetManager* assets = nullptr;
    std::function<Entity(const Uuid&)> entityOf;
};

struct SaveContext {
    const TypeRegistry& types;
    const AssetManager* assets = nullptr;
    std::function<std::optional<Uuid>(Entity)> uuidOf;
};

ENGINE_API std::string formatUuid(const Uuid& uuid);
ENGINE_API std::optional<Uuid> parseUuid(std::string_view text);
ENGINE_API Result<void> readYaml(const TypeInfo& type, void* object, const YamlNode& node, LoadContext& ctx);
ENGINE_API void writeYaml(const TypeInfo& type, const void* object, YamlWriter& out, SaveContext& ctx);

}
