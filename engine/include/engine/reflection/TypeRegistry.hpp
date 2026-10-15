/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TypeRegistry
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/ecs/IStorage.hpp"
#include "engine/reflection/TypeBuilder.hpp"
#include "engine/reflection/TypeInfo.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace engine {

class ENGINE_API TypeRegistry {
private:
    std::vector<TypeInfo> _types;
    std::unordered_map<uint64_t, ComponentId> _byKey;
    std::vector<const TypeInfo*> _messages;
    size_t _engineTypeCount = 0;

    size_t insert(TypeInfo info);
    void reindex();

public:
    template <class T>
    auto add();
    template <class T>
    ComponentId id() const;

    const TypeInfo* find(std::string_view name) const;
    const TypeInfo* findByKey(uint64_t key) const;
    const TypeInfo& info(ComponentId id) const;
    std::span<const TypeInfo> all() const;
    std::span<const TypeInfo* const> messages() const;
    uint64_t formatFingerprint() const;
    std::string closestName(std::string_view unknown) const;
    void beginGameTypes();
    void clearGameTypes();
};

}

#include "engine/reflection/TypeRegistry.inl"
