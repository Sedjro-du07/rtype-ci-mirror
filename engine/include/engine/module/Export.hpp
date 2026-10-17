/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Export
*/

#pragma once

#include "engine/module/IGameModule.hpp"
#include "engine/module/IPlugin.hpp"
#include "engine/module/StaticModuleTable.hpp"

#include <cstdint>

namespace engine {

inline constexpr uint32_t ENGINE_PLUGIN_API_VERSION = 1;
inline constexpr uint32_t ENGINE_GAME_API_VERSION = 1;

}

#if !defined(ENGINE_EXPORT)
#define ENGINE_EXPORT
#endif

#if !defined(ENGINE_MODULE_NAME)
#define ENGINE_MODULE_NAME ""
#endif

#define ENGINE_DETAIL_CREATE(Interface, ClassName)                                                                     \
    []() noexcept -> ::engine::Interface* {                                                                            \
        try {                                                                                                          \
            return new ClassName();                                                                                    \
        } catch (...) {                                                                                                \
            return nullptr;                                                                                            \
        }                                                                                                              \
    }

#if defined(ENGINE_STATIC_MODULES)

#define ENGINE_GAME_MODULE(ClassName)                                                                                  \
    namespace {                                                                                                        \
    const bool engineGameRegistered =                                                                                  \
        (::engine::StaticModuleTable::addGame(ENGINE_MODULE_NAME, ENGINE_DETAIL_CREATE(IGameModule, ClassName),        \
                                              [](::engine::IGameModule* game) noexcept { delete game; }),              \
         true);                                                                                                        \
    }

#define ENGINE_PLUGIN(ClassName)                                                                                       \
    namespace {                                                                                                        \
    const bool enginePluginRegistered =                                                                                \
        (::engine::StaticModuleTable::addPlugin(ENGINE_MODULE_NAME, ENGINE_DETAIL_CREATE(IPlugin, ClassName),          \
                                                [](::engine::IPlugin* plugin) noexcept { delete plugin; }),            \
         true);                                                                                                        \
    }

#else

#define ENGINE_GAME_MODULE(ClassName)                                                                                  \
    extern "C" ENGINE_EXPORT ::engine::IGameModule* create_game() noexcept {                                           \
        return ENGINE_DETAIL_CREATE(IGameModule, ClassName)();                                                         \
    }                                                                                                                  \
    extern "C" ENGINE_EXPORT void destroy_game(::engine::IGameModule* game) noexcept {                                 \
        delete game;                                                                                                   \
    }                                                                                                                  \
    extern "C" ENGINE_EXPORT uint32_t game_api_version() noexcept {                                                    \
        return ::engine::ENGINE_GAME_API_VERSION;                                                                      \
    }

#define ENGINE_PLUGIN(ClassName)                                                                                       \
    extern "C" ENGINE_EXPORT ::engine::IPlugin* create_plugin() noexcept {                                             \
        return ENGINE_DETAIL_CREATE(IPlugin, ClassName)();                                                             \
    }                                                                                                                  \
    extern "C" ENGINE_EXPORT void destroy_plugin(::engine::IPlugin* plugin) noexcept {                                 \
        delete plugin;                                                                                                 \
    }                                                                                                                  \
    extern "C" ENGINE_EXPORT uint32_t plugin_api_version() noexcept {                                                  \
        return ::engine::ENGINE_PLUGIN_API_VERSION;                                                                    \
    }

#endif
