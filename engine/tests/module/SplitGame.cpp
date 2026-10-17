/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SplitGame
*/

#include "engine/module/Export.hpp"

#include <string_view>

namespace fixture {

class SplitGame final : public engine::IGameModule {
public:
    void registerTypes(engine::TypeRegistry&) override {}
    void registerSystems(engine::Role, engine::Scheduler&) override {}
    std::string_view firstScene() const override {
#if defined(ENGINE_GAME_SERVER)
        return "server";
#else
        return "client";
#endif
    }
};

}

ENGINE_GAME_MODULE(fixture::SplitGame)
