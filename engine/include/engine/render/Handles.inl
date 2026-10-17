/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Handles
*/

#pragma once

#include "engine/render/Handles.hpp"

namespace engine {

template <class Tag>
constexpr bool Handle<Tag>::valid() const noexcept {
    return this->id != 0;
}

}
