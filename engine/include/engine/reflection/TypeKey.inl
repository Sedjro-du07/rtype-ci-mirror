/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TypeKey
*/

#pragma once

#include "engine/reflection/TypeKey.hpp"

#include <cstdint>
#include <string_view>

namespace engine {

template <class T>
constexpr uint64_t typeKey() noexcept {
    return TypeName<T>::key;
}

namespace detail {

constexpr std::string_view lastNamePart(std::string_view qualified) noexcept {
    const size_t separator = qualified.rfind("::");
    return separator == std::string_view::npos ? qualified : qualified.substr(separator + 2);
}

}

}
