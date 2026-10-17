/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Commands
*/

#pragma once

#include "engine/ecs/Commands.hpp"

#include <utility>

namespace engine {

template <class T>
void Commands::add(Entity e, T value) {
    this->_queue.emplace_back([e, value = std::move(value)](Registry& registry) mutable {
        if (registry.alive(e) && !registry.has<T>(e)) registry.emplace<T>(e, std::move(value));
    });
}

template <class T>
void Commands::set(Entity e, T value) {
    this->_queue.emplace_back([e, value = std::move(value)](Registry& registry) mutable {
        if (!registry.alive(e)) return;
        if (registry.has<T>(e))
            registry.replace<T>(e, std::move(value));
        else
            registry.emplace<T>(e, std::move(value));
    });
}

template <class T>
void Commands::remove(Entity e) {
    this->_queue.emplace_back([e](Registry& registry) {
        if (registry.alive(e)) registry.remove<T>(e);
    });
}

}
