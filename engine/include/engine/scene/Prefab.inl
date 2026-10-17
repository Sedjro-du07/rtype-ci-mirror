/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Prefab
*/

#pragma once

#include "engine/reflection/TypeRegistry.hpp"
#include "engine/scene/Prefab.hpp"

#include <utility>

namespace engine {

template <class T>
ComponentData ComponentData::of(const TypeRegistry& types, T value) {
    ComponentData data(types.info(types.id<T>()));
    *static_cast<T*>(data.value()) = std::move(value);
    return data;
}

}
