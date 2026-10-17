/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** IGameModule
*/

#pragma once

#include "engine/Export.hpp"

#include <cstdint>
#include <string_view>

namespace engine {

class TypeRegistry;
class Scheduler;
class Context;
class LobbyContext;
class ServiceContext;
enum class Role : uint8_t;

class ENGINE_API IGameModule {
public:
    virtual ~IGameModule() = default;
    virtual void registerTypes(TypeRegistry& types) = 0;
    virtual void registerSystems(Role role, Scheduler& scheduler) = 0;
    virtual std::string_view firstScene() const = 0;
    virtual void onStart(Context& ctx);
    virtual void onLobbyStart(LobbyContext& ctx);
    virtual void onRoomStart(Context& ctx);
    virtual void onShutdown(ServiceContext& ctx);
};

}
