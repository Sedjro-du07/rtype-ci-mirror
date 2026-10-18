/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Engine
*/

#pragma once

#include "engine/Context.hpp"
#include "engine/EngineConfig.hpp"
#include "engine/Export.hpp"
#include "engine/assets/AssetManager.hpp"
#include "engine/core/Signal.hpp"
#include "engine/core/Time.hpp"
#include "engine/ecs/Commands.hpp"
#include "engine/events/EventBus.hpp"
#include "engine/input/InputMap.hpp"
#include "engine/module/ModuleManager.hpp"
#include "engine/net/NetworkSession.hpp"
#include "engine/reflection/TypeRegistry.hpp"
#include "engine/render/RenderQueue.hpp"
#include "engine/scene/SceneManager.hpp"
#include "engine/scheduler/Scheduler.hpp"
#include "engine/services/CVars.hpp"
#include "engine/services/Logger.hpp"

#include <atomic>
#include <memory>
#include <optional>
#include <string_view>

namespace engine {

class IAudio;
class IGameModule;
class INetwork;
class IPlatform;
class IScripting;
class Replication;

class ENGINE_API Engine {
private:
    EngineConfig _config;
    Logger _log;
    CVars _cvars;
    ModuleManager _modules;
    TypeRegistry _types;
    EventBus _events;
    AssetManager _assets;
    InputMap _input;
    std::optional<NetworkSession> _network;
    std::unique_ptr<Replication> _replication;
    Scheduler _scheduler;
    SceneManager _scenes;
    std::optional<Commands> _commands;
    RenderQueue _render;
    std::optional<Context> _context;
    Time _time;
    Clock _clock;
    Subscription _quitEvent;
    IPlatform* _platform = nullptr;
    IAudio* _audio = nullptr;
    INetwork* _transport = nullptr;
    IScripting* _scripting = nullptr;
    IGameModule* _game = nullptr;
    std::atomic_bool _quit = false;
    double _accumulator = 0;
    int _exitCode = 0;
    bool _booted = false;

    std::string_view pluginName(PluginKind kind) const;
    template <class I>
    I* plugin(PluginKind kind);
    void boot();
    void loadPlugins();
    void loadGame();
    void load();
    void startReplication();
    void buildSchedule();
    void openFirstScene();
    void makeContext();
    void loop();
    void frame(double elapsed);
    bool tick(double step);
    void endFrame();
    void shutdown();

public:
    explicit Engine(EngineConfig config);
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    ~Engine();

    int run();
    void requestQuit() noexcept;
    const Time& time() const;
};

}
