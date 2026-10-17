/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ModuleManager
*/

#pragma once

#include "engine/EngineConfig.hpp"
#include "engine/Export.hpp"
#include "engine/core/Error.hpp"
#include "engine/module/DynamicLibrary.hpp"
#include "engine/module/IGameModule.hpp"
#include "engine/module/IPlugin.hpp"
#include "engine/module/StaticModuleTable.hpp"
#include "engine/services/Logger.hpp"

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace engine {

struct LoadedModule {
    std::string stem;
    std::filesystem::path file;
    bool isGame = false;
};

class ENGINE_API ModuleManager {
private:
    struct Entry {
        std::optional<DynamicLibrary> library;
        IPlugin* plugin = nullptr;
        IGameModule* game = nullptr;
        StaticModuleTable::DestroyPlugin destroyPlugin = nullptr;
        StaticModuleTable::DestroyGame destroyGame = nullptr;
    };

    Logger _log;
    std::filesystem::path _dir;
    std::vector<Entry> _entries;
    std::vector<LoadedModule> _loaded;

    Result<IPlugin*> loadPluginAs(PluginKind kind, std::string_view name, const PluginContext& ctx);
    void release(size_t index) noexcept;

public:
    ModuleManager(Logger& log, std::filesystem::path executableDir);
    ModuleManager(const ModuleManager&) = delete;
    ModuleManager& operator=(const ModuleManager&) = delete;
    ~ModuleManager();

    template <class I>
    Result<I*> loadPlugin(PluginKind kind, std::string_view name, const PluginContext& ctx);
    Result<IGameModule*> loadGame(std::string_view name, Role role);
    void unloadGame();
    void unloadAll() noexcept;
    std::span<const LoadedModule> loaded() const;
};

ENGINE_API std::string_view pluginKindName(PluginKind kind);

}

#include "engine/module/ModuleManager.inl"
