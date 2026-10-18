/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Replication
*/

#include "net/Replication.hpp"

#include "engine/ecs/Registry.hpp"
#include "engine/input/InputMap.hpp"
#include "engine/net/NetComponents.hpp"
#include "engine/net/NetworkSession.hpp"
#include "engine/reflection/TypeRegistry.hpp"

namespace engine {

Replication::Replication(ReplicationParts parts, const NetConfig& config, Logger& log)
    : _network(&parts.network), _types(&parts.types), _input(&parts.input), _prefabs(parts.prefabs),
      _assets(parts.assets), _config(config), _log(log.tagged("replication")) {}

void Replication::refreshTypes() {
    const auto all = this->_types->all();
    if (all.size() == this->_typeCount) return;
    this->_typeCount = all.size();
    this->_replicated.clear();
    for (const TypeInfo& type : all)
        if (type.category == TypeCategory::Component && (type.flags & Replicated)) this->_replicated.push_back(&type);
}

WireContext Replication::wire(Registry& world) const {
    WireContext context;
    context.networkIdOf = [&world](Entity e) {
        const NetworkId* id = world.alive(e) ? world.tryGet<NetworkId>(e) : nullptr;
        return id ? id->value : 0u;
    };
    context.entityOf = [this](uint32_t id) { return this->_ids.find(id); };
    context.assets = this->_assets;
    return context;
}

uint64_t Replication::actionMask() const {
    const size_t count = this->_input->order().size();
    return count >= 64 ? ~uint64_t{0} : (uint64_t{1} << count) - 1;
}

void Replication::receive(const InboundMessage& message) {
    if (message.id == SnapshotMessage && !this->_network->isServer()) {
        this->_snapshots.emplace_back(message.payload.begin(), message.payload.end());
        return;
    }
    if (message.id == InputMessage && this->_network->isServer())
        this->receiveInput(this->_network->player(message.from), message.payload);
}

void Replication::setRoomSession(uint16_t roomSession) {
    this->_roomSession = roomSession;
}

uint16_t Replication::roomSession() const {
    return this->_roomSession;
}

NetworkIdMap& Replication::ids() {
    return this->_ids;
}

std::span<const TypeInfo* const> Replication::replicated() {
    this->refreshTypes();
    return this->_replicated;
}

void Replication::reset() {
    this->_ids.clear();
    this->_lastSnapshot.reset();
    this->_live.clear();
    this->_gone.clear();
    this->_inputs.clear();
    this->_snapshots.clear();
    this->_applied.clear();
    this->_destroyed.clear();
    this->_sent.clear();
    this->_lastInputProcessed = 0;
    this->_serverTick = 0;
    this->_synced = false;
}

}
