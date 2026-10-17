/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Commands
*/

#include "engine/ecs/Commands.hpp"

#include "engine/reflection/TypeInfo.hpp"

#include <cassert>
#include <utility>

namespace engine {

namespace {

class RawValue {
private:
    const TypeInfo* _info;
    std::vector<std::byte> _bytes;

public:
    RawValue(const TypeInfo& info, std::vector<std::byte> bytes) : _info(&info), _bytes(std::move(bytes)) {}
    RawValue(RawValue&& other) noexcept : _info(std::exchange(other._info, nullptr)), _bytes(std::move(other._bytes)) {}
    RawValue& operator=(RawValue&&) = delete;
    RawValue(const RawValue&) = delete;
    RawValue& operator=(const RawValue&) = delete;
    ~RawValue() {
        if (this->_info && this->_info->destroy) this->_info->destroy(this->_bytes.data());
    }

    const void* data() const { return this->_bytes.data(); }
};

}

Commands::Commands(Registry& registry) : _registry(&registry) {}

Entity Commands::spawn() {
    return this->_registry->create();
}

void Commands::destroy(Entity e) {
    this->_queue.emplace_back([e](Registry& registry) { registry.destroy(e); });
}

void Commands::setParent(Entity child, Entity parent, Vec2 localOffset) {
    this->_queue.emplace_back([child, parent, localOffset](Registry& registry) {
        if (!registry.alive(child)) return;
        if (!parent.isNull() && !registry.alive(parent)) return;
        registry.setParent(child, parent, localOffset);
    });
}

void Commands::addRaw(Entity e, ComponentId id, std::vector<std::byte> value) {
    const TypeInfo& info = this->_registry->types().info(id);
    assert(value.size() >= info.size);
    this->_queue.emplace_back([e, id, raw = RawValue(info, std::move(value))](Registry& registry) {
        if (registry.alive(e) && !registry.getRaw(e, id)) registry.emplaceRaw(e, id, raw.data());
    });
}

bool Commands::empty() const {
    return this->_queue.empty();
}

void Commands::apply() {
    std::vector<Command> pending;
    pending.swap(this->_queue);
    for (Command& command : pending)
        command(*this->_registry);
}

}
