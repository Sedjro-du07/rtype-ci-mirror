/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** IScripting
*/

#pragma once

#include "engine/core/Error.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/module/IPlugin.hpp"
#include "engine/render/Handles.hpp"

#include <filesystem>
#include <string_view>

namespace engine {

class TypeRegistry;
class AssetManager;
class EventBus;
class Logger;
class Context;

struct ScriptHost {
    TypeRegistry& types;
    AssetManager& assets;
    EventBus& events;
    Logger& log;
};

class IScripting : public IPlugin {
public:
    virtual Result<void> bind(ScriptHost& host) = 0;
    virtual Result<ScriptHandle> load(const std::filesystem::path& file) = 0;
    virtual void unload(ScriptHandle script) = 0;
    virtual Result<void> reload(ScriptHandle script) = 0;
    virtual Result<void> call(ScriptHandle script, std::string_view function, Entity self, Context& ctx) = 0;
    virtual bool has(ScriptHandle script, std::string_view function) const = 0;
};

}
