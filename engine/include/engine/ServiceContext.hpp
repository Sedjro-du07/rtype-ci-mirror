/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ServiceContext
*/

#pragma once

#include "engine/EngineConfig.hpp"
#include "engine/Export.hpp"

namespace engine {

class AssetManager;
class Logger;

class ENGINE_API ServiceContext {
private:
    Logger* _log;
    AssetManager* _assets;
    Role _role;

public:
    ServiceContext(Logger& log, AssetManager& assets, Role role);

    Logger& log();
    AssetManager& assets();
    Role role() const;
};

}
