/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** AnimationSystem
*/

#include "engine/components/Animation.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/ecs/Registry.hpp"
#include "systems/EngineSystems.hpp"

namespace engine {

void animation(Registry& world, double dt) {
    for (auto [e, anim, sprite] : world.view<Animation, Sprite>()) {
        if (anim.frames.empty()) continue;
        const auto last = static_cast<uint32_t>(anim.frames.size() - 1);
        if (anim.current > last) anim.current = last;
        if (anim.fps > 0) {
            const float frameTime = 1.f / anim.fps;
            anim.time += static_cast<float>(dt);
            while (anim.time >= frameTime) {
                anim.time -= frameTime;
                if (anim.current < last)
                    ++anim.current;
                else if (anim.loop)
                    anim.current = 0;
                else
                    anim.time = 0;
            }
        }
        sprite.frame = anim.frames[anim.current];
    }
}

}
