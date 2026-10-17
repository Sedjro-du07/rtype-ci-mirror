/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TypeRegistry
*/

#pragma once

#include "engine/ecs/SparseSet.hpp"
#include "engine/reflection/TypeRegistry.hpp"

#include <cassert>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

namespace engine {

template <class T>
auto TypeRegistry::add() {
    static_assert(HasTypeName<T>, "TypeRegistry::add<T>(): T has no type macro. Place ENGINE_COMPONENT, "
                                  "ENGINE_EVENT, ENGINE_MESSAGE, ENGINE_STRUCT or ENGINE_ENUM right after T, "
                                  "outside any namespace.");
    using Name = TypeName<T>;

    TypeInfo info;
    info.key = Name::key;
    info.name = std::string(Name::value);
    info.category = Name::category;
    info.size = sizeof(T);
    info.align = alignof(T);
    if constexpr (std::is_default_constructible_v<T>) info.construct = [](void* dst) { ::new (dst) T(); };
    info.destroy = [](void* obj) noexcept { static_cast<T*>(obj)->~T(); };
    if constexpr (std::is_copy_constructible_v<T>)
        info.copy = [](void* dst, const void* src) { ::new (dst) T(*static_cast<const T*>(src)); };
    if constexpr (Name::category == TypeCategory::Component)
        info.makeStorage = [](ComponentId id) -> std::unique_ptr<IStorage> {
            return std::make_unique<SparseSet<T>>(id);
        };

    const size_t index = this->insert(std::move(info));

    if constexpr (Name::category == TypeCategory::Component)
        return ComponentBuilder<T>(this->_types, index);
    else if constexpr (Name::category == TypeCategory::Event)
        return EventBuilder<T>(this->_types, index);
    else if constexpr (Name::category == TypeCategory::Message)
        return MessageBuilder<T>(this->_types, index);
    else if constexpr (Name::category == TypeCategory::Struct)
        return StructBuilder<T>(this->_types, index);
    else
        return EnumBuilder<T>{};
}

template <class T>
ComponentId TypeRegistry::id() const {
    const TypeInfo* info = this->findByKey(typeKey<T>());
    assert(info && "TypeRegistry::id<T>(): T is not registered");
    return info->id;
}

}
