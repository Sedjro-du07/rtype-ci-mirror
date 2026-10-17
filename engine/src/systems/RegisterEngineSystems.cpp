/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RegisterEngineSystems
*/

#include "engine/scheduler/Scheduler.hpp"
#include "systems/EngineSystems.hpp"

#include <memory>
#include <utility>

namespace engine {

void registerEngineSystems(Scheduler& scheduler, TargetsOf targetsOf) {
    const auto of = std::make_shared<TargetsOf>(std::move(targetsOf));
    const auto system = [of](auto body) {
        return [of, body](Context& ctx) {
            SystemTargets targets = (*of)(ctx);
            body(targets);
        };
    };
    const uint8_t drawing = ClientRole | EditorRole;

    scheduler.add(Stage::FixedUpdate, "engine.previous_position",
                  system([](SystemTargets& t) { previousPosition(t.world); }));
    scheduler
        .add(Stage::FixedUpdate, "engine.movement",
             system([](SystemTargets& t) { movement(t.world, t.time.fixedDelta); }))
        .after("engine.previous_position");
    scheduler.add(Stage::FixedUpdate, "engine.hierarchy", system([](SystemTargets& t) { hierarchy(t.world); }))
        .after("engine.movement");
    scheduler.add(Stage::FixedUpdate, "engine.parallax", system([](SystemTargets& t) { parallax(t.world); }))
        .after("engine.movement")
        .roles(ClientRole);
    scheduler.add(Stage::FixedUpdate, "engine.lifetime",
                  system([](SystemTargets& t) { lifetime(t.world, t.commands, t.time.fixedDelta); }));
    scheduler
        .add(Stage::FixedUpdate, "engine.animation",
             system([](SystemTargets& t) { animation(t.world, t.time.fixedDelta); }))
        .roles(drawing);
    scheduler
        .add(Stage::FixedUpdate, "engine.collision",
             system([](SystemTargets& t) { collision(t.world, t.events, t.gridCell); }))
        .after("engine.hierarchy");

    scheduler
        .add(Stage::Render, "engine.render.sprites", system([](SystemTargets& t) {
                 if (t.render) renderSprites(t.world, *t.render, t.time.alpha);
             }))
        .roles(drawing);
    scheduler
        .add(Stage::Render, "engine.render.text", system([](SystemTargets& t) {
                 if (t.render) renderText(t.world, *t.render);
             }))
        .after("engine.render.sprites")
        .roles(drawing);
    scheduler
        .add(Stage::Render, "engine.render.debug", system([](SystemTargets& t) {
                 if (t.render && t.debugColliders) renderDebug(t.world, *t.render);
             }))
        .roles(drawing);
}

}
