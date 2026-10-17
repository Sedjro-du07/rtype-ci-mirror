/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Identity
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace engine {

struct Name {
    std::string value;
};

struct ENGINE_API Uuid {
    uint64_t hi = 0, lo = 0;

    static Uuid generate();
    bool operator==(const Uuid&) const = default;
};

struct ENGINE_API Tags {
    std::vector<uint64_t> ids;

    bool has(std::string_view tag) const;
};

}

template <>
struct ENGINE_API std::hash<engine::Uuid> {
    size_t operator()(const engine::Uuid& uuid) const noexcept;
};

ENGINE_COMPONENT(engine::Name)
ENGINE_COMPONENT(engine::Uuid)
ENGINE_COMPONENT(engine::Tags)
