/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ModuleManager
*/

#pragma once

#include "engine/module/ModuleManager.hpp"

#include <type_traits>

namespace engine {

template <class I>
Result<I*> ModuleManager::loadPlugin(PluginKind kind, std::string_view name, const PluginContext& ctx) {
    static_assert(std::is_base_of_v<IPlugin, I>, "loadPlugin<I>: I is a plugin interface (IPlatform, IAudio…)");
    auto plugin = this->loadPluginAs(kind, name, ctx);
    if (!plugin) return unexpected(plugin.error());
    return static_cast<I*>(*plugin);
}

}
