/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** OldApiPlugin
*/

#include "engine/module/Export.hpp"

#include <cstdint>

extern "C" ENGINE_EXPORT engine::IPlugin* create_plugin() noexcept {
    return nullptr;
}

extern "C" ENGINE_EXPORT void destroy_plugin(engine::IPlugin*) noexcept {}

extern "C" ENGINE_EXPORT uint32_t plugin_api_version() noexcept {
    return engine::ENGINE_PLUGIN_API_VERSION + 1;
}
