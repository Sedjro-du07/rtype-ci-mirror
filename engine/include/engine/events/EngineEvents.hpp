/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EngineEvents
*/

#pragma once

#include "engine/ecs/Entity.hpp"
#include "engine/net/NetTypes.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <cstdint>
#include <string>

namespace engine {

struct QuitRequested {};

struct WindowResized {
    uint32_t width = 0, height = 0;
};

struct WindowFocusChanged {
    bool focused = true;
};

struct AppSuspended {};

struct AppResumed {};

struct Collision {
    Entity a, b;
};

struct SceneLoaded {
    std::string name;
};

struct SceneUnloading {
    std::string name;
};

struct Caption {
    std::string text;
    float seconds = 0;
};

struct ConsoleCommand {
    std::string line;
};

struct PlayerJoined {
    PlayerId player = 0;
};

struct PlayerLeft {
    PlayerId player = 0;
    DisconnectReason reason = DisconnectReason::Quit;
    std::string message;
};

struct PlayerDisconnected {
    PlayerId player = 0;
    DisconnectReason reason = DisconnectReason::ConnectionLost;
};

struct PlayerReconnected {
    PlayerId player = 0;
};

struct JoinedServer {
    PlayerId me = 0;
};

struct LeftServer {
    DisconnectReason reason = DisconnectReason::Quit;
    std::string message;
};

struct ConnectionInterrupted {
    DisconnectReason reason = DisconnectReason::ConnectionLost;
};

struct Reconnected {};

}

ENGINE_EVENT(engine::QuitRequested)
ENGINE_EVENT(engine::WindowResized)
ENGINE_EVENT(engine::WindowFocusChanged)
ENGINE_EVENT(engine::AppSuspended)
ENGINE_EVENT(engine::AppResumed)
ENGINE_EVENT(engine::Collision)
ENGINE_EVENT(engine::SceneLoaded)
ENGINE_EVENT(engine::SceneUnloading)
ENGINE_EVENT(engine::Caption)
ENGINE_EVENT(engine::ConsoleCommand)
ENGINE_EVENT(engine::PlayerJoined)
ENGINE_EVENT(engine::PlayerLeft)
ENGINE_EVENT(engine::PlayerDisconnected)
ENGINE_EVENT(engine::PlayerReconnected)
ENGINE_EVENT(engine::JoinedServer)
ENGINE_EVENT(engine::LeftServer)
ENGINE_EVENT(engine::ConnectionInterrupted)
ENGINE_EVENT(engine::Reconnected)
