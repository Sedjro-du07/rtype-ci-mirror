/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** IPlugin
*/

#pragma once

#include "engine/core/Error.hpp"

#include <cstdint>
#include <string_view>

namespace engine {

class Logger;
class CVars;
struct EngineConfig;
class EventBus;

enum class PluginKind : uint8_t { Platform, Audio, Network, Scripting };

struct PluginContext {
    Logger& log;
    CVars& cvars;
    const EngineConfig& config;
    EventBus& events;
};

class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual PluginKind kind() const = 0;
    virtual std::string_view name() const = 0;
    virtual uint32_t apiVersion() const = 0;
    virtual Result<void> init(const PluginContext& ctx) = 0;
    virtual void shutdown() noexcept = 0;
};

}
