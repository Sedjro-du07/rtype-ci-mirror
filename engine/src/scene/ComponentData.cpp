/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ComponentData
*/

#include "engine/scene/Prefab.hpp"

#include <cassert>
#include <new>
#include <utility>

namespace engine {

void ComponentData::release() noexcept {
    if (!this->_value) return;
    this->_type->destroy(this->_value);
    ::operator delete(this->_value, std::align_val_t(this->_type->align));
    this->_value = nullptr;
}

ComponentData::ComponentData(const TypeInfo& type) : _type(&type) {
    this->_value = ::operator new(type.size, std::align_val_t(type.align));
    type.construct(this->_value);
}

ComponentData::ComponentData(const ComponentData& other) : _type(other._type), _source(other._source) {
    if (!other._value) return;
    this->_value = ::operator new(this->_type->size, std::align_val_t(this->_type->align));
    this->_type->copy(this->_value, other._value);
}

ComponentData::ComponentData(ComponentData&& other) noexcept
    : _type(std::exchange(other._type, nullptr)), _value(std::exchange(other._value, nullptr)),
      _source(std::move(other._source)) {}

ComponentData& ComponentData::operator=(ComponentData other) noexcept {
    std::swap(this->_type, other._type);
    std::swap(this->_value, other._value);
    std::swap(this->_source, other._source);
    return *this;
}

ComponentData::~ComponentData() {
    this->release();
}

Result<ComponentData> ComponentData::parse(const TypeInfo& type, const YamlNode& node, LoadContext& ctx) {
    ComponentData data(type);
    if (auto ok = readYaml(type, data._value, node, ctx); !ok) return unexpected(ok.error());
    data._source = node;
    return data;
}

const TypeInfo& ComponentData::type() const {
    assert(this->_type);
    return *this->_type;
}

void* ComponentData::value() {
    return this->_value;
}

const void* ComponentData::value() const {
    return this->_value;
}

const YamlNode& ComponentData::source() const {
    return this->_source;
}

}
