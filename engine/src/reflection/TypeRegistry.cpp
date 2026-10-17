/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TypeRegistry
*/

#include "engine/reflection/TypeRegistry.hpp"

#include "engine/core/Error.hpp"
#include "engine/core/Hash.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <string>
#include <utility>
#include <vector>

namespace engine {

namespace {

std::string lowered(std::string_view text) {
    std::string out(text);
    for (char& c : out)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

size_t editDistance(const std::string& a, const std::string& b) {
    std::vector<size_t> previous(b.size() + 1), current(b.size() + 1);
    for (size_t j = 0; j <= b.size(); ++j)
        previous[j] = j;
    for (size_t i = 1; i <= a.size(); ++i) {
        current[0] = i;
        for (size_t j = 1; j <= b.size(); ++j) {
            const size_t substitution = previous[j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1);
            current[j] = std::min({previous[j] + 1, current[j - 1] + 1, substitution});
        }
        std::swap(previous, current);
    }
    return previous[b.size()];
}

bool shapesTheWire(const TypeInfo& type) {
    return type.category == TypeCategory::Message ||
           (type.category == TypeCategory::Component && (type.flags & Replicated));
}

}

size_t TypeRegistry::insert(TypeInfo info) {
    if (const auto it = this->_byKey.find(info.key); it != this->_byKey.end()) {
        const std::string& existing = this->_types[it->second].name;
        const std::string message = existing == info.name
                                        ? "type '" + info.name + "' is registered twice"
                                        : "types '" + existing + "' and '" + info.name + "' have the same key";
        throw BootError(3, Error{Errc::DuplicateName, message});
    }
    const size_t index = this->_types.size();
    info.id = static_cast<ComponentId>(index);
    this->_byKey.emplace(info.key, info.id);
    this->_types.push_back(std::move(info));
    this->reindex();
    return index;
}

void TypeRegistry::reindex() {
    this->_messages.clear();
    for (const TypeInfo& type : this->_types)
        if (type.category == TypeCategory::Message) this->_messages.push_back(&type);
}

const TypeInfo* TypeRegistry::find(std::string_view name) const {
    const TypeInfo* type = this->findByKey(fnv1a64(name));
    return type && type->name == name ? type : nullptr;
}

const TypeInfo* TypeRegistry::findByKey(uint64_t key) const {
    const auto it = this->_byKey.find(key);
    return it == this->_byKey.end() ? nullptr : &this->_types[it->second];
}

const TypeInfo& TypeRegistry::info(ComponentId id) const {
    assert(id < this->_types.size());
    return this->_types[id];
}

std::span<const TypeInfo> TypeRegistry::all() const {
    return this->_types;
}

std::span<const TypeInfo* const> TypeRegistry::messages() const {
    return this->_messages;
}

uint64_t TypeRegistry::formatFingerprint() const {
    std::string description;
    for (const TypeInfo& type : this->_types) {
        if (!shapesTheWire(type)) continue;
        description += type.name;
        description += '(';
        description += std::to_string(static_cast<int>(type.transport));
        for (const FieldInfo& field : type.fields) {
            description += ';' + field.name + ':' + std::to_string(static_cast<int>(field.kind)) + ':' +
                           std::to_string(static_cast<int>(field.element)) + ':' + std::to_string(field.elementType) +
                           ':' + std::to_string(field.maxLength) + ':' + std::to_string(field.quantMin) + ':' +
                           std::to_string(field.quantMax);
        }
        description += ')';
    }
    return fnv1a64(description);
}

std::string TypeRegistry::closestName(std::string_view unknown) const {
    const std::string wanted = lowered(unknown);
    const size_t limit = std::max<size_t>(1, wanted.size() / 3);
    std::string best;
    size_t bestDistance = limit + 1;
    for (const TypeInfo& type : this->_types) {
        const size_t distance = editDistance(wanted, lowered(type.name));
        if (distance < bestDistance) {
            bestDistance = distance;
            best = type.name;
        }
    }
    return best;
}

void TypeRegistry::beginGameTypes() {
    this->_engineTypeCount = this->_types.size();
}

void TypeRegistry::clearGameTypes() {
    this->_types.erase(this->_types.begin() + static_cast<std::ptrdiff_t>(this->_engineTypeCount), this->_types.end());
    this->_byKey.clear();
    for (const TypeInfo& type : this->_types)
        this->_byKey.emplace(type.key, type.id);
    this->reindex();
}

}
