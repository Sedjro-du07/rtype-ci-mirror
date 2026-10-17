/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Context
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/ServiceContext.hpp"
#include "engine/core/Time.hpp"

#include <functional>

namespace engine {

class ActionState;
class Commands;
class EventBus;
class IAudio;
class InputMap;
class NetworkSession;
class Registry;
class RenderQueue;
class SceneManager;

class ENGINE_API Context : public ServiceContext {
private:
    struct Parts {
        SceneManager& scenes;
        Commands& commands;
        EventBus& events;
        const Time& time;
        InputMap& input;
        RenderQueue& render;
        IAudio& audio;
        NetworkSession* network;
        std::function<void()> quit;
    };

    SceneManager* _scenes;
    Commands* _commands;
    EventBus* _events;
    const Time* _time;
    InputMap* _input;
    RenderQueue* _render;
    IAudio* _audio;
    NetworkSession* _network;
    std::function<void()> _quit;

    Context(ServiceContext services, Parts parts);

    friend class Engine;
    friend class RoomRuntime;

public:
    Registry& world();
    Commands& commands();
    SceneManager& scenes();
    EventBus& events();
    const Time& time() const;
    const ActionState& input() const;
    InputMap& inputMap();
    RenderQueue& render();
    IAudio& audio();
    NetworkSession& network();
    bool online() const;
    void requestQuit();
};

}
