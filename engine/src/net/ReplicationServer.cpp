/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ReplicationServer
*/

#include "engine/core/Time.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/input/InputMap.hpp"
#include "engine/net/NetComponents.hpp"
#include "engine/net/NetworkSession.hpp"
#include "engine/reflection/ByteStream.hpp"
#include "net/Replication.hpp"
#include "reflection/Codec.hpp"

#include <algorithm>
#include <cmath>

namespace engine {

namespace {

constexpr size_t MaxSnapshotBytes = 1024;
constexpr size_t HeaderBytes = 12;
constexpr size_t MaxPendingInputs = 4;

void append(std::vector<std::byte>& out, std::span<const std::byte> bytes) {
    out.insert(out.end(), bytes.begin(), bytes.end());
}

}

void Replication::assignIds(Registry& world) {
    std::vector<Entity> fresh;
    for (auto [e, prefab] : world.view<NetPrefab>(exclude<NetworkId>))
        fresh.push_back(e);
    for (Entity e : fresh) {
        const uint32_t id = this->_ids.allocate();
        world.emplace<NetworkId>(e, NetworkId{id});
        this->_ids.bind(id, e);
    }
}

bool Replication::due(double now) {
    const double period = this->_config.snapshotRate > 0 ? 1.0 / this->_config.snapshotRate : 0.0;
    if (this->_lastSnapshot && now - *this->_lastSnapshot < period - 1e-9) return false;
    this->_lastSnapshot = now;
    return true;
}

void Replication::sendSnapshots(Registry& world, const Time& time) {
    if (!this->_network->isServer() || !this->due(time.total)) return;
    this->refreshTypes();
    const WireContext wire = this->wire(world);
    auto entries = this->spawnEntries(world, wire);
    this->trackDestroyed(world);
    this->appendDestroys(entries);
    for (ConnectionId connection : this->_network->connections())
        this->sendEntries(connection, static_cast<uint32_t>(time.tick), entries);
}

std::vector<std::byte> Replication::encodeEntity(Registry& world, Entity e, const WireContext& wire) const {
    std::vector<std::byte> body;
    uint8_t count = 0;
    for (size_t index = 0; index < this->_replicated.size(); ++index) {
        const TypeInfo& type = *this->_replicated[index];
        const void* raw = world.getRaw(e, type.id);
        if (!raw) continue;
        std::vector<std::byte> fields;
        ByteWriter out(fields, MaxSnapshotBytes);
        codec::encode(type, raw, out, wire);
        if (out.overflowed()) continue;
        std::vector<std::byte> prefix;
        ByteWriter head(prefix, 4);
        head.write(static_cast<uint16_t>(index));
        head.write(static_cast<uint16_t>(fields.size()));
        append(body, prefix);
        append(body, fields);
        ++count;
    }
    std::vector<std::byte> entry;
    ByteWriter out(entry, 10);
    out.write(world.get<NetworkId>(e).value);
    out.write(static_cast<uint8_t>(Spawn));
    out.write(world.get<NetPrefab>(e).prefabKey);
    out.write(count);
    append(entry, body);
    return entry;
}

std::vector<std::vector<std::byte>> Replication::spawnEntries(Registry& world, const WireContext& wire) const {
    std::vector<std::vector<std::byte>> entries;
    for (auto [e, id, prefab] : world.view<NetworkId, NetPrefab>())
        entries.push_back(this->encodeEntity(world, e, wire));
    return entries;
}

void Replication::trackDestroyed(Registry& world) {
    std::unordered_set<uint32_t> live;
    for (auto [e, id] : world.view<NetworkId>())
        live.insert(id.value);
    const auto resends = static_cast<uint32_t>(std::max(1.0, std::ceil(this->_config.snapshotRate)));
    for (uint32_t id : this->_live)
        if (!live.contains(id)) {
            this->_gone.push_back(Gone{id, resends});
            this->_ids.unbind(id);
        }
    this->_live = std::move(live);
}

void Replication::appendDestroys(std::vector<std::vector<std::byte>>& entries) {
    for (Gone& gone : this->_gone) {
        std::vector<std::byte> entry;
        ByteWriter out(entry, 5);
        out.write(gone.id);
        out.write(static_cast<uint8_t>(Destroy));
        entries.push_back(std::move(entry));
        --gone.resends;
    }
    std::erase_if(this->_gone, [](const Gone& gone) { return gone.resends == 0; });
}

void Replication::sendEntries(ConnectionId to, uint32_t tick, std::span<const std::vector<std::byte>> entries) {
    const auto player = this->_inputs.find(this->_network->player(to));
    Header header{this->_roomSession, tick, player == this->_inputs.end() ? 0u : player->second.processed, 0};
    size_t first = 0, used = HeaderBytes;
    for (size_t i = 0; i < entries.size(); ++i) {
        if (entries[i].size() + HeaderBytes > MaxSnapshotBytes) {
            this->_log.error("an entity needs {} bytes, more than one snapshot message", entries[i].size());
            continue;
        }
        if (used + entries[i].size() <= MaxSnapshotBytes) {
            used += entries[i].size();
            continue;
        }
        this->sendMessage(to, header, entries.subspan(first, i - first));
        first = i;
        used = HeaderBytes + entries[i].size();
    }
    this->sendMessage(to, header, entries.subspan(first));
}

void Replication::sendMessage(ConnectionId to, const Header& header, std::span<const std::vector<std::byte>> entries) {
    std::vector<std::byte> payload;
    ByteWriter out(payload, HeaderBytes);
    out.write(header.roomSession);
    out.write(header.tick);
    out.write(header.lastInputProcessed);
    const size_t at = payload.size();
    out.write(uint16_t{0});
    uint16_t count = 0;
    for (const auto& entry : entries) {
        if (entry.size() + HeaderBytes > MaxSnapshotBytes) continue;
        append(payload, entry);
        ++count;
    }
    payload[at] = static_cast<std::byte>(count >> 8);
    payload[at + 1] = static_cast<std::byte>(count & 0xFF);
    this->_network->sendRaw(to, Transport::Unreliable, SnapshotMessage, payload);
}

void Replication::receiveInput(PlayerId player, std::span<const std::byte> payload) {
    ByteReader in(payload);
    const auto session = in.u16();
    const auto newest = in.u32();
    const auto count = in.u8();
    if (!session || !newest || !count || *session != this->_roomSession || *count < 1 || *count > 4) return;
    std::vector<uint64_t> masks;
    for (uint8_t i = 0; i < *count; ++i)
        if (const auto mask = in.u64()) masks.push_back(*mask & this->actionMask());
    if (masks.size() != *count) return;
    PlayerInputs& inputs = this->_inputs[player];
    for (uint32_t i = 0; i < masks.size() && i < *newest; ++i)
        if (*newest - i > inputs.processed) inputs.pending.emplace(*newest - i, masks[i]);
    while (inputs.pending.size() > MaxPendingInputs)
        inputs.pending.erase(inputs.pending.begin());
}

void Replication::consumeInput(PlayerInputs& inputs) {
    inputs.previousMask = inputs.lastMask;
    if (inputs.pending.empty()) return;
    const auto oldest = inputs.pending.begin();
    inputs.processed = oldest->first;
    inputs.lastMask = oldest->second;
    inputs.pending.erase(oldest);
}

void Replication::applyInputs(Registry& world) {
    for (auto& [player, inputs] : this->_inputs)
        this->consumeInput(inputs);
    std::vector<std::pair<Entity, RemoteInput>> updates;
    for (auto [e, owner] : world.view<NetOwner>()) {
        const auto found = this->_inputs.find(owner.owner);
        if (found == this->_inputs.end()) continue;
        const PlayerInputs& inputs = found->second;
        updates.emplace_back(
            e, RemoteInput{ActionState::fromMask(inputs.lastMask, inputs.previousMask, this->_input->order()),
                           inputs.lastMask, inputs.processed});
    }
    for (auto& [e, input] : updates) {
        if (RemoteInput* current = world.tryGet<RemoteInput>(e)) {
            *current = std::move(input);
            continue;
        }
        world.emplace<RemoteInput>(e, std::move(input));
    }
}

uint32_t Replication::lastInputProcessed(PlayerId player) const {
    const auto found = this->_inputs.find(player);
    return found == this->_inputs.end() ? 0 : found->second.processed;
}

}
