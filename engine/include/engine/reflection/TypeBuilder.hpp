/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TypeBuilder
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/reflection/TypeInfo.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace engine {

using FromYamlFn = Result<void> (*)(void* obj, const YamlNode& node, LoadContext& ctx);
using ToYamlFn = void (*)(const void* obj, YamlWriter& out, SaveContext& ctx);
using EncodeFn = void (*)(const void* obj, ByteWriter& out, const WireContext& wire);
using DecodeFn = Result<void> (*)(void* obj, ByteReader& in, const WireContext& wire);

template <class M>
struct FieldKindOf {};

template <class M>
concept HasFieldKind = requires { FieldKindOf<M>::value; };

ENGINE_API EnumInfo makeEnumInfo(std::string_view names, std::span<const int64_t> values);

template <class E>
const EnumInfo& enumInfoOf();

template <class T>
class FieldsBuilder {
private:
    std::vector<TypeInfo>* _types;
    size_t _index;

protected:
    TypeInfo& info();

public:
    FieldsBuilder(std::vector<TypeInfo>& types, size_t index);

    template <class Self, class M>
    Self&& field(this Self&& self, std::string_view name, M T::*member, std::string_view tooltip = {});
    template <class Self>
    Self&& maxLength(this Self&& self, uint32_t n);
    template <class Self>
    Self&& quantize(this Self&& self, float min, float max);
};

template <class T>
class ComponentBuilder : public FieldsBuilder<T> {
public:
    using FieldsBuilder<T>::FieldsBuilder;

    ComponentBuilder& flags(uint32_t f);
    ComponentBuilder& yaml(FromYamlFn read, ToYamlFn write);
    ComponentBuilder& binary(EncodeFn encode, DecodeFn decode);
};

template <class T>
class MessageBuilder : public FieldsBuilder<T> {
public:
    using FieldsBuilder<T>::FieldsBuilder;

    MessageBuilder& unreliable();
    MessageBuilder& reliable();
    MessageBuilder& stream();
    MessageBuilder& binary(EncodeFn encode, DecodeFn decode);
};

template <class T>
class EventBuilder : public FieldsBuilder<T> {
public:
    using FieldsBuilder<T>::FieldsBuilder;
};

template <class T>
class StructBuilder : public FieldsBuilder<T> {
public:
    using FieldsBuilder<T>::FieldsBuilder;
};

template <class T>
class EnumBuilder {};

}

#include "engine/reflection/TypeBuilder.inl"
