/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ThrowingGame
*/

#include "engine/module/Export.hpp"

#include <stdexcept>
#include <string_view>

namespace fixture {

class ThrowingGame final : public engine::IGameModule {
public:
    ThrowingGame() { throw std::runtime_error("the game failed to start"); }
    void registerTypes(engine::TypeRegistry&) override {}
    void registerSystems(engine::Role, engine::Scheduler&) override {}
    std::string_view firstScene() const override { return {}; }
};

}

ENGINE_GAME_MODULE(fixture::ThrowingGame)
