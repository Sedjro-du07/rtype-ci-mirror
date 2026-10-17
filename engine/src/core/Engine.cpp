/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Engine
*/

#include "engine/Engine.hpp"

#include "engine/audio/IAudio.hpp"
#include "engine/events/EngineEvents.hpp"
#include "engine/module/IGameModule.hpp"
#include "engine/net/INetwork.hpp"
#include "engine/render/IPlatform.hpp"
#include "engine/script/IScripting.hpp"
#include "os/Os.hpp"
#include "systems/EngineSystems.hpp"
#include "systems/RegisterEngineTypes.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <filesystem>
#include <format>
#include <thread>
#include <utility>

namespace engine {

namespace {

constexpr double MaxFrame = 0.25;
constexpr int ExitConfig = 1;
constexpr int ExitPlugin = 2;
constexpr int ExitGame = 3;
constexpr int ExitScene = 4;

[[noreturn]] void bootFailure(int code, const Error& error) {
    throw BootError(code, error);
}

}

Engine::Engine(EngineConfig config)
    : _config(std::move(config)), _log(_config.log), _modules(_log, os::executableDir()), _assets(_log, _config.root),
      _scheduler(_config.role, _log), _scenes(_types, _assets, _events, _log) {}

Engine::~Engine() {
    this->shutdown();
}

std::string_view Engine::pluginName(PluginKind kind) const {
    const bool server = this->_config.role == Role::Server;
    switch (kind) {
    case PluginKind::Platform:
        return server ? "null" : std::string_view(this->_config.platform);
    case PluginKind::Audio:
        return server ? "null" : std::string_view(this->_config.audio);
    case PluginKind::Network:
        return this->_config.network.empty() ? "null" : std::string_view(this->_config.network);
    case PluginKind::Scripting:
        break;
    }
    return this->_config.scripting.empty() ? "null" : std::string_view(this->_config.scripting);
}

template <class I>
I* Engine::plugin(PluginKind kind) {
    const PluginContext ctx{this->_log, this->_cvars, this->_config, this->_events};
    auto loaded = this->_modules.loadPlugin<I>(kind, this->pluginName(kind), ctx);
    if (!loaded) bootFailure(ExitPlugin, loaded.error());
    return *loaded;
}

void Engine::loadPlugins() {
    this->_platform = this->plugin<IPlatform>(PluginKind::Platform);
    this->_audio = this->plugin<IAudio>(PluginKind::Audio);
    this->_transport = this->plugin<INetwork>(PluginKind::Network);
    this->_scripting = this->plugin<IScripting>(PluginKind::Scripting);
    this->_assets.attach(this->_platform->renderer(), *this->_audio, *this->_scripting);
    this->_network.emplace(this->_events, this->_log, this->_config.net, this->_types);
    this->_network->attach(*this->_transport);
    if (this->_config.role != Role::Server) return;
    if (auto opened = this->_network->open(true, this->_config.net.port); !opened)
        bootFailure(ExitPlugin, opened.error());
}

void Engine::loadGame() {
    if (this->_config.game.empty())
        bootFailure(ExitGame, Error{Errc::GameModuleInvalid, "no game module: set 'game' in the configuration"});
    auto game = this->_modules.loadGame(this->_config.game, this->_config.role);
    if (!game) bootFailure(ExitGame, game.error());
    this->_game = *game;
    registerEngineTypes(this->_types);
    this->_types.beginGameTypes();
    this->_game->registerTypes(this->_types);
}

void Engine::boot() {
    this->_log.info("starting {} as {}", this->_config.game, roleName(this->_config.role));
    os::onQuitSignal([this] { this->requestQuit(); });
    this->loadPlugins();
    this->loadGame();
    this->_booted = true;
}

void Engine::buildSchedule() {
    registerEngineSystems(this->_scheduler, [this](Context& ctx) {
        const bool drawing = this->_config.role != Role::Server;
        return SystemTargets{ctx.world(), ctx.commands(), ctx.events(), ctx.time(), drawing ? &ctx.render() : nullptr,
                             64.f,        false};
    });
    this->_scheduler.beginGameSystems();
    this->_game->registerSystems(this->_config.role, this->_scheduler);
    if (auto built = this->_scheduler.build(); !built) bootFailure(ExitGame, built.error());
}

void Engine::openFirstScene() {
    const std::filesystem::path scene = this->_config.sceneOverride.empty()
                                            ? std::filesystem::path(this->_game->firstScene())
                                            : this->_config.sceneOverride;
    if (scene.empty()) return;
    if (auto loaded = this->_scenes.load(scene); !loaded) bootFailure(ExitScene, loaded.error());
}

void Engine::makeContext() {
    this->_commands.emplace(this->_scenes.world(), &this->_scenes.prefabs());
    Context::Parts parts{this->_scenes, *this->_commands, this->_events,
                         this->_time,   this->_input,     this->_render,
                         *this->_audio, &*this->_network, [this] { this->requestQuit(); }};
    this->_context.emplace(Context(ServiceContext(this->_log, this->_assets, this->_config.role), std::move(parts)));
}

void Engine::load() {
    this->_time.fixedDelta = 1.0 / this->_config.tickRate;
    this->buildSchedule();
    const std::filesystem::path bindings = this->_config.root / "input.yaml";
    if (std::error_code error; std::filesystem::is_regular_file(bindings, error))
        if (auto ok = this->_input.load(bindings); !ok) bootFailure(ExitConfig, ok.error());
    if (this->_config.role != Role::Server)
        if (auto opened = this->_platform->openWindow(this->_config.window); !opened)
            bootFailure(ExitPlugin, opened.error());
    this->_quitEvent = this->_events.subscribe<QuitRequested>([this](const QuitRequested&) { this->requestQuit(); });
    this->_events.beginGameSubscriptions();
    this->openFirstScene();
    this->makeContext();
    if (this->_config.role == Role::Server)
        this->_game->onRoomStart(*this->_context);
    else
        this->_game->onStart(*this->_context);
    this->_commands->apply();
    this->_events.dispatch();
}

bool Engine::tick(double step) {
    this->_input.beginTick(step);
    this->_scheduler.run(Stage::FixedUpdate, *this->_context);
    this->_commands->apply();
    this->_events.dispatch();
    this->_accumulator -= step;
    ++this->_time.tick;
    this->_time.total += step;
    if (this->_config.maxTicks != 0 && this->_time.tick >= this->_config.maxTicks) this->requestQuit();
    return !this->_quit && !this->_scheduler.quitRequested();
}

void Engine::endFrame() {
    this->_scheduler.run(Stage::LateUpdate, *this->_context);
    this->_commands->apply();
    this->_events.dispatch();
    this->_network->flush(this->_time.total);
    if (this->_scenes.hasPending()) {
        this->_scenes.applyPending();
        this->_commands.emplace(this->_scenes.world(), &this->_scenes.prefabs());
        this->_context->_commands = &*this->_commands;
    }
    this->_time.alpha = this->_accumulator / this->_time.fixedDelta;
    if (this->_config.role != Role::Server) {
        this->_scheduler.run(Stage::Render, *this->_context);
        this->_platform->renderer().submit(this->_render.frame());
        this->_render.clear();
    }
    this->_audio->update();
}

void Engine::frame(double elapsed) {
    this->_time.frameDelta = elapsed;
    this->_accumulator += std::min(elapsed, MaxFrame);
    this->_platform->pollEvents(this->_input, this->_events);
    this->_network->poll(this->_time.total);
    while (this->_accumulator >= this->_time.fixedDelta)
        if (!this->tick(this->_time.fixedDelta)) break;
    this->endFrame();
    if (this->_scheduler.quitRequested()) {
        this->_exitCode = this->_scheduler.exitCode();
        this->requestQuit();
    }
}

void Engine::loop() {
    const bool fixedTime = this->_config.maxTicks != 0;
    this->_clock.restart();
    while (!this->_quit) {
        const double elapsed = fixedTime ? this->_time.fixedDelta : this->_clock.restart();
        this->frame(elapsed);
        if (fixedTime || this->_config.role != Role::Server) continue;
        const double wait = this->_time.fixedDelta - this->_accumulator;
        if (wait > 0) std::this_thread::sleep_for(std::chrono::duration<double>(wait));
    }
}

void Engine::shutdown() {
    os::onQuitSignal([] {});
    if (!this->_booted) {
        this->_modules.unloadAll();
        return;
    }
    this->_booted = false;
    this->_log.info("shutting down after {} ticks", this->_time.tick);
    if (this->_context && this->_game) this->_game->onShutdown(*this->_context);
    this->_quitEvent = {};
    this->_context.reset();
    this->_commands.reset();
    this->_scenes.clear();
    this->_events.clearGameSubscriptions();
    this->_scheduler.clearGameSystems();
    this->_types.clearGameTypes();
    this->_modules.unloadGame();
    this->_game = nullptr;
    this->_network.reset();
    this->_modules.unloadAll();
    this->_log.flush();
}

int Engine::run() {
    try {
        this->boot();
        this->load();
        this->loop();
    } catch (const BootError& error) {
        this->_log.error("{}", error.error().toString());
        this->_exitCode = error.exitCode();
    } catch (const std::exception& error) {
        this->_log.error("game module failed: {}", error.what());
        this->_exitCode = ExitGame;
    }
    this->shutdown();
    return this->_exitCode;
}

void Engine::requestQuit() noexcept {
    this->_quit = true;
}

const Time& Engine::time() const {
    return this->_time;
}

}
