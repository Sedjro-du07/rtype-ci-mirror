/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Replication
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/net/NetTypes.hpp"
#include "engine/net/NetworkIdMap.hpp"
#include "engine/reflection/TypeInfo.hpp"
#include "engine/services/Logger.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace engine {

class ActionState;
class AssetManager;
class ByteReader;
class InputMap;
class NetworkSession;
class PrefabLibrary;
class Registry;
class TypeRegistry;
struct InboundMessage;
struct Time;

inline constexpr uint16_t SnapshotMessage = 0x0060;
inline constexpr uint16_t InputMessage = 0x0061;

struct ReplicationParts {
    NetworkSession& network;
    const TypeRegistry& types;
    const InputMap& input;
    PrefabLibrary* prefabs = nullptr;
    AssetManager* assets = nullptr;
};

class ENGINE_API Replication {
private:
    enum Op : uint8_t { Update = 0, Spawn = 1, Destroy = 2 };

    struct Component {
        uint16_t index = 0;
        std::span<const std::byte> bytes;
    };

    struct Entry {
        uint32_t id = 0;
        uint8_t op = Update;
        uint32_t prefabKey = 0;
        std::vector<Component> components;
    };

    struct Header {
        uint16_t roomSession = 0;
        uint32_t tick = 0;
        uint32_t lastInputProcessed = 0;
        uint16_t count = 0;
    };

    struct Gone {
        uint32_t id = 0;
        uint32_t resends = 0;
    };

    struct PlayerInputs {
        std::map<uint32_t, uint64_t> pending;
        uint32_t processed = 0;
        uint64_t lastMask = 0;
        uint64_t previousMask = 0;
    };

    NetworkSession* _network;
    const TypeRegistry* _types;
    const InputMap* _input;
    PrefabLibrary* _prefabs;
    AssetManager* _assets;
    NetConfig _config;
    Logger _log;
    NetworkIdMap _ids;
    std::vector<const TypeInfo*> _replicated;
    size_t _typeCount = 0;
    uint16_t _roomSession = 0;

    std::optional<double> _lastSnapshot;
    std::unordered_set<uint32_t> _live;
    std::vector<Gone> _gone;
    std::unordered_map<PlayerId, PlayerInputs> _inputs;

    std::vector<std::vector<std::byte>> _snapshots;
    std::unordered_map<uint32_t, uint32_t> _applied;
    std::unordered_set<uint32_t> _destroyed;
    std::unordered_set<uint32_t> _unknownPrefabs;
    std::deque<uint64_t> _sent;
    uint32_t _inputSeq = 0;
    uint32_t _lastInputProcessed = 0;
    uint32_t _serverTick = 0;
    double _serverTime = 0;
    double _receivedAt = 0;
    bool _synced = false;

    void refreshTypes();
    WireContext wire(Registry& world) const;
    uint64_t actionMask() const;

    bool due(double now);
    std::vector<std::byte> encodeEntity(Registry& world, Entity e, const WireContext& wire) const;
    std::vector<std::vector<std::byte>> spawnEntries(Registry& world, const WireContext& wire) const;
    void trackDestroyed(Registry& world);
    void appendDestroys(std::vector<std::vector<std::byte>>& entries);
    void sendEntries(ConnectionId to, uint32_t tick, std::span<const std::vector<std::byte>> entries);
    void sendMessage(ConnectionId to, const Header& header, std::span<const std::vector<std::byte>> entries);
    void receiveInput(PlayerId player, std::span<const std::byte> payload);
    void consumeInput(PlayerInputs& inputs);

    void applySnapshot(Registry& world, std::span<const std::byte> payload, const Time& time);
    std::optional<Header> readHeader(ByteReader& in) const;
    std::optional<Entry> readEntry(ByteReader& in) const;
    void syncClock(uint32_t tick, const Time& time);
    void applyEntry(Registry& world, const Entry& entry, uint32_t tick, double at, const WireContext& wire);
    bool fresh(uint32_t id, uint32_t tick) const;
    void destroyEntity(Registry& world, uint32_t id);
    Entity spawnEntity(Registry& world, uint32_t id, uint32_t prefabKey);
    void writeComponent(Registry& world, Entity e, const Component& component, double at, const WireContext& wire);
    void trackPosition(Registry& world, Entity e, double at);

public:
    Replication(ReplicationParts parts, const NetConfig& config, Logger& log);
    Replication(const Replication&) = delete;
    Replication& operator=(const Replication&) = delete;

    void receive(const InboundMessage& message);
    void setRoomSession(uint16_t roomSession);
    uint16_t roomSession() const;
    NetworkIdMap& ids();
    std::span<const TypeInfo* const> replicated();
    void reset();

    void assignIds(Registry& world);
    void sendSnapshots(Registry& world, const Time& time);
    void applyInputs(Registry& world);
    uint32_t lastInputProcessed(PlayerId player) const;

    void applySnapshots(Registry& world, const Time& time);
    void interpolate(Registry& world, const Time& time);
    void sendInput(const ActionState& state);
    uint32_t lastInputProcessed() const;
    double serverTime(const Time& time) const;
};

}
