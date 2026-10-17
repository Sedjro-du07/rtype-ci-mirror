/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TypeKey
*/

#pragma once

#include "engine/core/Hash.hpp"

#include <array>
#include <cstdint>
#include <string_view>

namespace engine {

enum class TypeCategory : uint8_t { Component, Event, Message, Struct, Enum };

template <class T>
struct TypeName;

template <class T>
concept HasTypeName = requires { TypeName<T>::key; };

template <class T>
constexpr uint64_t typeKey() noexcept;

namespace detail {

constexpr std::string_view lastNamePart(std::string_view qualified) noexcept;

}

}

#define ENGINE_DETAIL_TYPE_NAME(Type, Category)                                                                        \
    template <>                                                                                                        \
    struct engine::TypeName<Type> {                                                                                    \
        static constexpr std::string_view value = engine::detail::lastNamePart(#Type);                                 \
        static constexpr uint64_t key = engine::fnv1a64(value);                                                        \
        static constexpr engine::TypeCategory category = engine::TypeCategory::Category;                               \
    };

#define ENGINE_COMPONENT(Type) ENGINE_DETAIL_TYPE_NAME(Type, Component)
#define ENGINE_EVENT(Type) ENGINE_DETAIL_TYPE_NAME(Type, Event)
#define ENGINE_MESSAGE(Type) ENGINE_DETAIL_TYPE_NAME(Type, Message)
#define ENGINE_STRUCT(Type) ENGINE_DETAIL_TYPE_NAME(Type, Struct)

#define ENGINE_ENUM(Type, ...)                                                                                         \
    template <>                                                                                                        \
    struct engine::TypeName<Type> {                                                                                    \
        static constexpr std::string_view value = engine::detail::lastNamePart(#Type);                                 \
        static constexpr uint64_t key = engine::fnv1a64(value);                                                        \
        static constexpr engine::TypeCategory category = engine::TypeCategory::Enum;                                   \
        static constexpr std::string_view valueNames = #__VA_ARGS__;                                                   \
        static constexpr auto values = [] {                                                                            \
            using enum Type;                                                                                           \
            return std::array{__VA_ARGS__};                                                                            \
        }();                                                                                                           \
    };

#include "engine/reflection/TypeKey.inl"
