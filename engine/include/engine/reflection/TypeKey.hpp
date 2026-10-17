/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TypeKey
*/
// ENG-22 — TypeKey, TypeName, TypeCategory and type macros (minimal subset needed by SRV-08)
// Spec: docs/engine/reference/04-reflection.md · ADR 0027, 0036 · reference 04

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace engine {

enum class TypeCategory : uint8_t { Component, Event, Message, Struct, Enum };

template <class T>
struct TypeName;

template <class T>
concept HasTypeName = requires { TypeName<T>::key; };

namespace detail {

constexpr std::string_view lastNamePart(std::string_view qualified) noexcept
{
    const auto pos = qualified.rfind(':');
    return pos == std::string_view::npos ? qualified : qualified.substr(pos + 1);
}

constexpr uint64_t fnv1a64(std::string_view text) noexcept
{
    uint64_t hash = 0xcbf29ce484222325ull;
    for (unsigned char c : text) {
        hash ^= c;
        hash *= 0x00000100000001B3ull;
    }
    return hash;
}

} // namespace detail

template <class T>
constexpr uint64_t typeKey() noexcept
{
    return TypeName<T>::key;
}

} // namespace engine

// ---------------------------------------------------------------------------
// Type registration macros — place right after the type definition, outside
// any namespace (ADR 0036).
// ---------------------------------------------------------------------------

#define ENGINE_DETAIL_TYPE_NAME(Type, Category)                          \
    template <>                                                          \
    struct engine::TypeName<Type> {                                      \
        static constexpr std::string_view value =                        \
            engine::detail::lastNamePart(#Type);                         \
        static constexpr uint64_t key = engine::detail::fnv1a64(value); \
        static constexpr engine::TypeCategory category =                 \
            engine::TypeCategory::Category;                              \
    }

#define ENGINE_COMPONENT(Type) ENGINE_DETAIL_TYPE_NAME(Type, Component)
#define ENGINE_EVENT(Type)     ENGINE_DETAIL_TYPE_NAME(Type, Event)
#define ENGINE_MESSAGE(Type)   ENGINE_DETAIL_TYPE_NAME(Type, Message)
#define ENGINE_STRUCT(Type)    ENGINE_DETAIL_TYPE_NAME(Type, Struct)

#define ENGINE_ENUM(Type, ...)                                                    \
    template <>                                                                   \
    struct engine::TypeName<Type> {                                               \
        static constexpr std::string_view value =                                 \
            engine::detail::lastNamePart(#Type);                                  \
        static constexpr uint64_t key = engine::detail::fnv1a64(value);          \
        static constexpr engine::TypeCategory category = engine::TypeCategory::Enum; \
        static constexpr std::string_view valueNames = #__VA_ARGS__;             \
    }
