/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ServiceContext
*/

#include "engine/ServiceContext.hpp"

namespace engine {

ServiceContext::ServiceContext(Logger& log, AssetManager& assets, Role role)
    : _log(&log), _assets(&assets), _role(role) {}

Logger& ServiceContext::log() {
    return *this->_log;
}

AssetManager& ServiceContext::assets() {
    return *this->_assets;
}

Role ServiceContext::role() const {
    return this->_role;
}

}
