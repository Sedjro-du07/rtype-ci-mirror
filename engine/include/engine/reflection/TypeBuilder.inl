/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TypeBuilder
*/

#pragma once

#include "engine/core/math/Color.hpp"
#include "engine/core/math/Rect.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/reflection/TypeBuilder.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace engine {

template <>
struct FieldKindOf<bool> {
    static constexpr FieldKind value = FieldKind::Bool;
};

template <>
struct FieldKindOf<int8_t> {
    static constexpr FieldKind value = FieldKind::Int8;
};

template <>
struct FieldKindOf<uint8_t> {
    static constexpr FieldKind value = FieldKind::UInt8;
};

template <>
struct FieldKindOf<int16_t> {
    static constexpr FieldKind value = FieldKind::Int16;
};

template <>
struct FieldKindOf<uint16_t> {
    static constexpr FieldKind value = FieldKind::UInt16;
};

template <>
struct FieldKindOf<int32_t> {
    static constexpr FieldKind value = FieldKind::Int32;
};

template <>
struct FieldKindOf<uint32_t> {
    static constexpr FieldKind value = FieldKind::UInt32;
};

template <>
struct FieldKindOf<int64_t> {
    static constexpr FieldKind value = FieldKind::Int64;
};

template <>
struct FieldKindOf<float> {
    static constexpr FieldKind value = FieldKind::Float;
};

template <>
struct FieldKindOf<double> {
    static constexpr FieldKind value = FieldKind::Double;
};

template <>
struct FieldKindOf<std::string> {
    static constexpr FieldKind value = FieldKind::String;
};

template <>
struct FieldKindOf<Vec2> {
    static constexpr FieldKind value = FieldKind::Vec2;
};

template <>
struct FieldKindOf<Rect> {
    static constexpr FieldKind value = FieldKind::Rect;
};

template <>
struct FieldKindOf<Color> {
    static constexpr FieldKind value = FieldKind::Color;
};

template <>
struct FieldKindOf<Entity> {
    static constexpr FieldKind value = FieldKind::Entity;
};

namespace detail {

template <class>
inline constexpr bool alwaysFalse = false;

template <class M>
struct ListOf {
    static constexpr bool value = false;
};

template <class E, class A>
struct ListOf<std::vector<E, A>> {
    static constexpr bool value = true;
    using Element = E;
};

template <class M>
constexpr bool isRegistered(TypeCategory category) {
    if constexpr (HasTypeName<M>)
        return TypeName<M>::category == category;
    else
        return false;
}

template <class M>
constexpr FieldKind kindOf() {
    if constexpr (HasFieldKind<M>)
        return FieldKindOf<M>::value;
    else if constexpr (isRegistered<M>(TypeCategory::Enum))
        return FieldKind::Enum;
    else if constexpr (isRegistered<M>(TypeCategory::Struct))
        return FieldKind::Struct;
    else
        static_assert(
            alwaysFalse<M>,
            "unsupported field type: use bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, "
            "float, double, "
            "std::string, Vec2, Rect, Color, Entity, an asset handle, a type registered "
            "with ENGINE_STRUCT or ENGINE_ENUM, or a std::vector of one of them");
}

template <class M>
void describe(FieldInfo& field) {
    field.kind = kindOf<M>();
    if constexpr (isRegistered<M>(TypeCategory::Enum)) field.enumInfo = &enumInfoOf<M>();
    if constexpr (isRegistered<M>(TypeCategory::Struct)) field.elementType = typeKey<M>();
}

template <class T, class M>
size_t offsetOf(M T::*member) {
    static_assert(std::is_default_constructible_v<T>, "a type with fields must be default-constructible");
    const T object{};
    const auto* base = reinterpret_cast<const std::byte*>(std::addressof(object));
    const auto* field = reinterpret_cast<const std::byte*>(std::addressof(object.*member));
    return static_cast<size_t>(field - base);
}

}

template <class E>
const EnumInfo& enumInfoOf() {
    static_assert(std::is_enum_v<E>, "enumInfoOf<E>() needs an enum registered with ENGINE_ENUM");
    static const EnumInfo info = [] {
        constexpr auto& values = TypeName<E>::values;
        std::array<int64_t, values.size()> numbers{};
        for (size_t i = 0; i < values.size(); ++i)
            numbers[i] = static_cast<int64_t>(std::to_underlying(values[i]));
        return makeEnumInfo(TypeName<E>::valueNames, numbers);
    }();
    return info;
}

template <class T>
FieldsBuilder<T>::FieldsBuilder(std::vector<TypeInfo>& types, size_t index) : _types(&types), _index(index) {}

template <class T>
TypeInfo& FieldsBuilder<T>::info() {
    return (*this->_types)[this->_index];
}

template <class T>
template <class Self, class M>
Self&& FieldsBuilder<T>::field(this Self&& self, std::string_view name, M T::*member, std::string_view tooltip) {
    FieldsBuilder<T>& base = self;
    FieldInfo field;
    field.name = std::string(name);
    field.tooltip = std::string(tooltip);
    field.offset = detail::offsetOf(member);
    if constexpr (detail::ListOf<M>::value) {
        using Element = typename detail::ListOf<M>::Element;
        detail::describe<Element>(field);
        field.element = field.kind;
        field.kind = FieldKind::List;
    } else {
        detail::describe<M>(field);
    }
    base.info().fields.push_back(std::move(field));
    return std::forward<Self>(self);
}

template <class T>
template <class Self>
Self&& FieldsBuilder<T>::maxLength(this Self&& self, uint32_t n) {
    FieldsBuilder<T>& base = self;
    auto& fields = base.info().fields;
    assert(!fields.empty() && (fields.back().kind == FieldKind::String || fields.back().kind == FieldKind::List));
    fields.back().maxLength = n;
    return std::forward<Self>(self);
}

template <class T>
template <class Self>
Self&& FieldsBuilder<T>::quantize(this Self&& self, float min, float max) {
    FieldsBuilder<T>& base = self;
    auto& fields = base.info().fields;
    assert(!fields.empty() && (fields.back().kind == FieldKind::Float || fields.back().kind == FieldKind::Vec2));
    assert(min < max);
    fields.back().quantMin = min;
    fields.back().quantMax = max;
    return std::forward<Self>(self);
}

template <class T>
ComponentBuilder<T>& ComponentBuilder<T>::flags(uint32_t f) {
    this->info().flags = f;
    return *this;
}

template <class T>
ComponentBuilder<T>& ComponentBuilder<T>::yaml(FromYamlFn read, ToYamlFn write) {
    this->info().fromYaml = read;
    this->info().toYaml = write;
    return *this;
}

template <class T>
ComponentBuilder<T>& ComponentBuilder<T>::binary(EncodeFn encode, DecodeFn decode) {
    this->info().encode = encode;
    this->info().decode = decode;
    return *this;
}

template <class T>
MessageBuilder<T>& MessageBuilder<T>::unreliable() {
    this->info().transport = Transport::Unreliable;
    return *this;
}

template <class T>
MessageBuilder<T>& MessageBuilder<T>::reliable() {
    this->info().transport = Transport::Reliable;
    return *this;
}

template <class T>
MessageBuilder<T>& MessageBuilder<T>::stream() {
    this->info().transport = Transport::Stream;
    return *this;
}

template <class T>
MessageBuilder<T>& MessageBuilder<T>::binary(EncodeFn encode, DecodeFn decode) {
    this->info().encode = encode;
    this->info().decode = decode;
    return *this;
}

}
