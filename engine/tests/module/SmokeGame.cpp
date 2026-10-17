/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SmokeGame
*/

#include "engine/Context.hpp"
#include "engine/components/Position.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/module/Export.hpp"
#include "engine/reflection/TypeRegistry.hpp"
#include "engine/scheduler/Scheduler.hpp"

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace smoke {

struct Marker {
    int32_t id = 0;
};

}

ENGINE_COMPONENT(smoke::Marker)

namespace smoke {

constexpr uint64_t CheckTick = 60;

void check(engine::Context& ctx) {
    if (ctx.time().tick + 1 != CheckTick) return;
    for (auto [e, marker, position] : ctx.world().view<Marker, engine::Position>()) {
        if (std::fabs(position.x - 60.f) > 0.5f) throw std::runtime_error("the marker did not move one second");
        ctx.requestQuit();
        return;
    }
    throw std::runtime_error("the scene has no marker");
}

class SmokeGame final : public engine::IGameModule {
public:
    void registerTypes(engine::TypeRegistry& types) override { types.add<Marker>().field("id", &Marker::id); }
    void registerSystems(engine::Role, engine::Scheduler& scheduler) override {
        scheduler.add(engine::Stage::FixedUpdate, "smoke.check", check).after("engine.movement");
    }
    std::string_view firstScene() const override { return "scenes/smoke.yaml"; }
};

}

ENGINE_GAME_MODULE(smoke::SmokeGame)
