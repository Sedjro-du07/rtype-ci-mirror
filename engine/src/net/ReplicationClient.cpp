/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ReplicationClient
*/

#include "engine/components/Position.hpp"
#include "engine/core/Time.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/input/InputMap.hpp"
#include "engine/net/NetComponents.hpp"
#include "engine/net/NetworkSession.hpp"
#include "engine/reflection/ByteStream.hpp"
#include "engine/reflection/TypeRegistry.hpp"
#include "engine/scene/Prefab.hpp"
#include "net/Replication.hpp"
#include "reflection/Codec.hpp"

#include <algorithm>

namespace engine {

namespace {

constexpr size_t MaxSentInputs = 4;

template <class T>
void put(Registry& world, Entity e, T value) {
    if (T* current = world.tryGet<T>(e)) {
        *current = std::move(value);
        return;
    }
    world.emplace<T>(e, std::move(value));
}

}

void Replication::applySnapshots(Registry& world, const Time& time) {
    if (this->_network->isServer() || this->_snapshots.empty()) return;
    this->refreshTypes();
    auto snapshots = std::move(this->_snapshots);
    this->_snapshots.clear();
    for (const auto& payload : snapshots)
        this->applySnapshot(world, payload, time);
}

std::optional<Replication::Header> Replication::readHeader(ByteReader& in) const {
    const auto session = in.u16();
    const auto tick = in.u32();
    const auto last = in.u32();
    const auto count = in.u16();
    if (!session || !tick || !last || !count) return std::nullopt;
    return Header{*session, *tick, *last, *count};
}

void Replication::applySnapshot(Registry& world, std::span<const std::byte> payload, const Time& time) {
    ByteReader in(payload);
    const auto header = this->readHeader(in);
    if (!header || header->roomSession != this->_roomSession) return;
    this->_lastInputProcessed = std::max(this->_lastInputProcessed, header->lastInputProcessed);
    this->syncClock(header->tick, time);
    const WireContext wire = this->wire(world);
    const double at = header->tick * time.fixedDelta;
    for (uint16_t i = 0; i < header->count; ++i) {
        const auto entry = this->readEntry(in);
        if (!entry) return;
        this->applyEntry(world, *entry, header->tick, at, wire);
    }
}

std::optional<Replication::Entry> Replication::readEntry(ByteReader& in) const {
    Entry entry;
    const auto id = in.u32();
    const auto op = in.u8();
    if (!id || !op || *op > Destroy) return std::nullopt;
    entry.id = *id;
    entry.op = *op;
    if (entry.op == Destroy) return entry;
    const auto key = entry.op == Spawn ? in.u32() : std::optional<uint32_t>(0);
    const auto count = in.u8();
    if (!key || !count) return std::nullopt;
    entry.prefabKey = *key;
    for (uint8_t i = 0; i < *count; ++i) {
        const auto index = in.u16();
        const auto length = in.u16();
        const auto bytes = length ? in.bytes(*length) : std::nullopt;
        if (!index || !bytes) return std::nullopt;
        entry.components.push_back(Component{*index, *bytes});
    }
    return entry;
}

void Replication::syncClock(uint32_t tick, const Time& time) {
    if (this->_synced && tick <= this->_serverTick) return;
    const auto connections = this->_network->connections();
    const double rtt = connections.empty() ? 0.0 : this->_network->stats(connections.front()).rttMs;
    this->_serverTick = tick;
    this->_serverTime = tick * time.fixedDelta + rtt / 2000.0;
    this->_receivedAt = time.total;
    this->_synced = true;
}

bool Replication::fresh(uint32_t id, uint32_t tick) const {
    const auto last = this->_applied.find(id);
    return last == this->_applied.end() || tick > last->second;
}

void Replication::applyEntry(Registry& world, const Entry& entry, uint32_t tick, double at, const WireContext& wire) {
    if (this->_destroyed.contains(entry.id)) return;
    if (entry.op == Destroy) return this->destroyEntity(world, entry.id);
    if (!this->fresh(entry.id, tick)) return;
    Entity e = this->_ids.find(entry.id);
    if (e != Entity::null() && !world.alive(e)) e = Entity::null();
    if (e == Entity::null() && entry.op == Update) return;
    if (e == Entity::null()) e = this->spawnEntity(world, entry.id, entry.prefabKey);
    this->_applied[entry.id] = tick;
    for (const Component& component : entry.components)
        this->writeComponent(world, e, component, at, wire);
}

void Replication::destroyEntity(Registry& world, uint32_t id) {
    this->_destroyed.insert(id);
    this->_applied.erase(id);
    const Entity e = this->_ids.find(id);
    this->_ids.unbind(id);
    if (e != Entity::null() && world.alive(e)) world.destroy(e);
}

Entity Replication::spawnEntity(Registry& world, uint32_t id, uint32_t prefabKey) {
    const Prefab* prefab = this->_prefabs ? this->_prefabs->findByKey(prefabKey) : nullptr;
    if (!prefab && this->_unknownPrefabs.insert(prefabKey).second)
        this->_log.warn("no loaded prefab has key {:08x}: its entities spawn empty", prefabKey);
    const Entity e = prefab ? this->_prefabs->instantiate(world, *prefab, {}) : world.create();
    put(world, e, NetworkId{id});
    put(world, e, NetPrefab{prefabKey});
    this->_ids.bind(id, e);
    return e;
}

void Replication::writeComponent(Registry& world, Entity e, const Component& component, double at,
                                 const WireContext& wire) {
    if (component.index >= this->_replicated.size()) return;
    const TypeInfo& type = *this->_replicated[component.index];
    ComponentData value(type);
    ByteReader in(component.bytes);
    if (!codec::decode(type, value.value(), in, wire)) return;
    if (void* current = world.getRaw(e, type.id)) {
        type.destroy(current);
        type.copy(current, value.value());
    } else {
        world.emplaceRaw(e, type.id, value.value());
    }
    if (type.id == this->_types->id<Position>()) this->trackPosition(world, e, at);
}

void Replication::trackPosition(Registry& world, Entity e, double at) {
    const Position& position = world.get<Position>(e);
    const Vec2 now{position.x, position.y};
    if (NetInterpolation* interpolation = world.tryGet<NetInterpolation>(e)) {
        *interpolation = NetInterpolation{interpolation->to, now, interpolation->toTime, at};
        return;
    }
    world.emplace<NetInterpolation>(e, NetInterpolation{now, now, at, at});
    if (PreviousPosition* previous = world.tryGet<PreviousPosition>(e)) *previous = PreviousPosition{now.x, now.y};
}

double Replication::serverTime(const Time& time) const {
    return this->_serverTime + (time.total - this->_receivedAt);
}

void Replication::interpolate(Registry& world, const Time& time) {
    if (!this->_synced) return;
    const double renderTime = this->serverTime(time) - this->_config.interpolationDelay;
    for (auto [e, interpolation, position] : world.view<NetInterpolation, Position>()) {
        const double span = interpolation.toTime - interpolation.fromTime;
        const double t = span > 0 ? std::clamp((renderTime - interpolation.fromTime) / span, 0.0, 1.0) : 1.0;
        position.x = static_cast<float>(interpolation.from.x + (interpolation.to.x - interpolation.from.x) * t);
        position.y = static_cast<float>(interpolation.from.y + (interpolation.to.y - interpolation.from.y) * t);
        if (PreviousPosition* previous = world.tryGet<PreviousPosition>(e))
            *previous = PreviousPosition{position.x, position.y};
    }
}

void Replication::sendInput(const ActionState& state) {
    if (this->_network->isServer()) return;
    const auto connections = this->_network->connections();
    if (connections.empty()) return;
    const size_t keep = std::min<size_t>(MaxSentInputs, this->_config.inputRedundancy + 1);
    this->_sent.push_front(state.toMask(this->_input->order()));
    while (this->_sent.size() > keep)
        this->_sent.pop_back();
    std::vector<std::byte> payload;
    ByteWriter out(payload, 7 + 8 * MaxSentInputs);
    out.write(this->_roomSession);
    out.write(++this->_inputSeq);
    out.write(static_cast<uint8_t>(this->_sent.size()));
    for (uint64_t mask : this->_sent)
        out.write(mask);
    this->_network->sendRaw(connections.front(), Transport::Unreliable, InputMessage, payload);
}

uint32_t Replication::lastInputProcessed() const {
    return this->_lastInputProcessed;
}

}
