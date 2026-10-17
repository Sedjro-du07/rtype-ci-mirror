/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** FixtureGame
*/

#include "engine/module/Export.hpp"

#include <string_view>

namespace fixture {

class FixtureGame final : public engine::IGameModule {
public:
    void registerTypes(engine::TypeRegistry&) override {}
    void registerSystems(engine::Role, engine::Scheduler&) override {}
    std::string_view firstScene() const override { return "scenes/fixture.scene.yaml"; }
};

}

ENGINE_GAME_MODULE(fixture::FixtureGame)
