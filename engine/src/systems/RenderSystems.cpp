/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RenderSystems
*/

#include "engine/components/Collider.hpp"
#include "engine/components/Parallax.hpp"
#include "engine/components/Position.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/components/Text.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/render/RenderQueue.hpp"
#include "systems/EngineSystems.hpp"

namespace engine {

namespace {

constexpr int16_t DebugLayer = 30000;

}

void renderSprites(Registry& world, RenderQueue& render, double alpha) {
    const auto t = static_cast<float>(alpha);
    const float travel = render.camera().center.x - Camera2D{}.center.x;
    for (auto [e, position, sprite] : world.view<Position, Sprite>()) {
        if (!sprite.visible) continue;
        Vec2 at{position.x, position.y};
        if (const auto* previous = world.tryGet<PreviousPosition>(e))
            at = {previous->x + (position.x - previous->x) * t, previous->y + (position.y - previous->y) * t};
        if (const auto* layer = world.tryGet<Parallax>(e)) at.x += travel * (1.f - layer->factor);
        render.sprite(sprite.texture, sprite.frame, at, sprite.layer, sprite.scale, sprite.rotation, sprite.tint,
                      sprite.origin);
    }
}

void renderText(Registry& world, RenderQueue& render) {
    for (auto [e, position, text] : world.view<Position, Text>())
        render.text(text.value, text.font, text.size, {position.x, position.y}, text.layer, text.color,
                    text.screenSpace);
}

void renderDebug(Registry& world, RenderQueue& render) {
    for (auto [e, position, collider] : world.view<Position, Collider>()) {
        const Color color = collider.trigger ? Color{255, 220, 0, 255} : Color{0, 255, 0, 255};
        render.rect(worldBox(position, collider), color, DebugLayer, false);
    }
}

}
