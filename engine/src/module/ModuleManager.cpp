/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ModuleManager
*/

#include "engine/module/ModuleManager.hpp"

#include "engine/module/Export.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <ranges>
#include <string>
#include <utility>

namespace engine {

namespace {

using PluginVersion = uint32_t (*)() noexcept;
using GameVersion = uint32_t (*)() noexcept;

[[maybe_unused]] std::string_view roleName(Role role) {
    switch (role) {
    case Role::Server:
        return "server";
    case Role::Client:
    case Role::Editor:
        break;
    }
    return "client";
}

}

std::string_view pluginKindName(PluginKind kind) {
    switch (kind) {
    case PluginKind::Audio:
        return "audio";
    case PluginKind::Network:
        return "network";
    case PluginKind::Scripting:
        return "scripting";
    case PluginKind::Platform:
        break;
    }
    return "platform";
}

ModuleManager::ModuleManager(Logger& log, std::filesystem::path executableDir)
    : _log(log.tagged("modules")), _dir(std::move(executableDir)) {}

ModuleManager::~ModuleManager() {
    this->unloadAll();
}

Result<IPlugin*> ModuleManager::loadPluginAs(PluginKind kind, std::string_view name, const PluginContext& ctx) {
    const std::string stem = std::format("{}_{}", pluginKindName(kind), name);
    if (name == "null")
        return fail(Errc::PluginNotFound,
                    std::format("the null {} plugin is not available yet (ENG-15)", pluginKindName(kind)));
    Entry entry;
    LoadedModule info{stem, {}, false};
#if defined(ENGINE_STATIC_MODULES)
    const PluginEntry* found = StaticModuleTable::findPlugin(stem);
    if (!found) return fail(Errc::PluginNotFound, std::format("plugin '{}' is not linked into this executable", stem));
    entry.plugin = found->create();
    entry.destroyPlugin = found->destroy;
#else
    const std::filesystem::path file = this->_dir / "plugins" / DynamicLibrary::decorate(stem);
    auto library = DynamicLibrary::open(file);
    if (!library)
        return fail(Errc::PluginNotFound, std::format("plugin '{}': {}", stem, library.error().message),
                    file.generic_string());
    auto version = library->symbol<PluginVersion>("plugin_api_version");
    auto create = library->symbol<StaticModuleTable::CreatePlugin>("create_plugin");
    auto destroy = library->symbol<StaticModuleTable::DestroyPlugin>("destroy_plugin");
    if (!version || !create || !destroy)
        return fail(
            Errc::SymbolNotFound,
            std::format("plugin '{}' does not export create_plugin, destroy_plugin and plugin_api_version", stem),
            file.generic_string());
    if (const uint32_t built = (*version)(); built != ENGINE_PLUGIN_API_VERSION)
        return fail(Errc::ApiMismatch,
                    std::format("plugin '{}' was built for plugin API {}, the engine uses {}", stem, built,
                                ENGINE_PLUGIN_API_VERSION),
                    file.generic_string());
    entry.plugin = (*create)();
    entry.destroyPlugin = *destroy;
    info.file = file;
    entry.library = std::move(*library);
#endif
    if (!entry.plugin) return fail(Errc::PluginNotFound, std::format("plugin '{}' failed to create its object", stem));
    const auto reject = [&](Error error) -> Result<IPlugin*> {
        entry.destroyPlugin(entry.plugin);
        return unexpected(std::move(error));
    };
    if (entry.plugin->kind() != kind)
        return reject(
            Error{Errc::PluginNotFound, std::format("plugin '{}' is a {} plugin, expected {}", stem,
                                                    pluginKindName(entry.plugin->kind()), pluginKindName(kind))});
    if (entry.plugin->apiVersion() != ENGINE_PLUGIN_API_VERSION)
        return reject(Error{Errc::ApiMismatch, std::format("plugin '{}' reports API {}, the engine uses {}", stem,
                                                           entry.plugin->apiVersion(), ENGINE_PLUGIN_API_VERSION)});
    if (auto ok = entry.plugin->init(ctx); !ok) return reject(ok.error());

    IPlugin* plugin = entry.plugin;
    this->_entries.push_back(std::move(entry));
    this->_loaded.push_back(std::move(info));
    this->_log.info("loaded plugin {} ({})", stem, plugin->name());
    return plugin;
}

Result<IGameModule*> ModuleManager::loadGame(std::string_view name, Role role) {
    if (std::ranges::any_of(this->_loaded, &LoadedModule::isGame))
        return fail(Errc::GameModuleInvalid, "a game module is already loaded: unload it first");
    Entry entry;
    LoadedModule info{std::string(name), {}, true};
#if defined(ENGINE_STATIC_MODULES)
    (void)role;
    const GameEntry* found = StaticModuleTable::findGame(name);
    if (!found) return fail(Errc::GameModuleInvalid, std::format("game '{}' is not linked into this executable", name));
    entry.game = found->create();
    entry.destroyGame = found->destroy;
#else
    const std::filesystem::path folder = this->_dir / "games";
    std::filesystem::path file = folder / DynamicLibrary::decorate(std::format("{}_{}", name, roleName(role)));
    std::error_code error;
    if (!std::filesystem::is_regular_file(file, error)) file = folder / DynamicLibrary::decorate(name);
    auto library = DynamicLibrary::open(file);
    if (!library)
        return fail(Errc::GameModuleInvalid, std::format("game '{}': {}", name, library.error().message),
                    file.generic_string());
    auto version = library->symbol<GameVersion>("game_api_version");
    auto create = library->symbol<StaticModuleTable::CreateGame>("create_game");
    auto destroy = library->symbol<StaticModuleTable::DestroyGame>("destroy_game");
    if (!version || !create || !destroy)
        return fail(Errc::SymbolNotFound,
                    std::format("game '{}' does not export create_game, destroy_game and game_api_version", name),
                    file.generic_string());
    if (const uint32_t built = (*version)(); built != ENGINE_GAME_API_VERSION)
        return fail(Errc::ApiMismatch,
                    std::format("game '{}' was built for game API {}, the engine uses {}", name, built,
                                ENGINE_GAME_API_VERSION),
                    file.generic_string());
    entry.game = (*create)();
    entry.destroyGame = *destroy;
    info.file = file;
    entry.library = std::move(*library);
#endif
    if (!entry.game) return fail(Errc::GameModuleInvalid, std::format("game '{}' failed to create its module", name));
    IGameModule* game = entry.game;
    this->_entries.push_back(std::move(entry));
    this->_loaded.push_back(std::move(info));
    this->_log.info("loaded game {}{}", name,
                    this->_loaded.back().file.empty() ? "" : " from " + this->_loaded.back().file.filename().string());
    return game;
}

void ModuleManager::release(size_t index) noexcept {
    Entry& entry = this->_entries[index];
    const LoadedModule& info = this->_loaded[index];
    this->_log.info("unloading {} {}", info.isGame ? "game" : "plugin", info.stem);
    if (entry.plugin) {
        entry.plugin->shutdown();
        entry.destroyPlugin(entry.plugin);
    }
    if (entry.game) entry.destroyGame(entry.game);
    entry.library.reset();
    this->_entries.erase(this->_entries.begin() + static_cast<std::ptrdiff_t>(index));
    this->_loaded.erase(this->_loaded.begin() + static_cast<std::ptrdiff_t>(index));
}

void ModuleManager::unloadGame() {
    for (size_t i = this->_loaded.size(); i-- > 0;)
        if (this->_loaded[i].isGame) this->release(i);
}

void ModuleManager::unloadAll() noexcept {
    while (!this->_entries.empty())
        this->release(this->_entries.size() - 1);
}

std::span<const LoadedModule> ModuleManager::loaded() const {
    return this->_loaded;
}

}
