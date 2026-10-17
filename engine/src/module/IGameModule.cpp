/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** IGameModule
*/

#include "engine/module/IGameModule.hpp"

namespace engine {

void IGameModule::onStart(Context&) {}

void IGameModule::onLobbyStart(LobbyContext&) {}

void IGameModule::onRoomStart(Context&) {}

void IGameModule::onShutdown(ServiceContext&) {}

}
