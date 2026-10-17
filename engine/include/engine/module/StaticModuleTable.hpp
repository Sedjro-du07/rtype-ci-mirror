/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** StaticModuleTable
*/

#pragma once

#include "engine/Export.hpp"

#include <string_view>

namespace engine {

class IPlugin;
class IGameModule;
struct PluginEntry;
struct GameEntry;

class ENGINE_API StaticModuleTable {
public:
    using CreatePlugin = IPlugin* (*)() noexcept;
    using DestroyPlugin = void (*)(IPlugin*) noexcept;
    using CreateGame = IGameModule* (*)() noexcept;
    using DestroyGame = void (*)(IGameModule*) noexcept;

    static void addPlugin(std::string_view stem, CreatePlugin create, DestroyPlugin destroy);
    static void addGame(std::string_view name, CreateGame create, DestroyGame destroy);
    static const PluginEntry* findPlugin(std::string_view stem);
    static const GameEntry* findGame(std::string_view name);
};

struct PluginEntry {
    std::string_view stem;
    StaticModuleTable::CreatePlugin create;
    StaticModuleTable::DestroyPlugin destroy;
};

struct GameEntry {
    std::string_view name;
    StaticModuleTable::CreateGame create;
    StaticModuleTable::DestroyGame destroy;
};

}
