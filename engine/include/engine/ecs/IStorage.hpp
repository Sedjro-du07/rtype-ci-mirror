/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** IStorage
*/

#pragma once

#include "engine/ecs/Entity.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace engine {

using ComponentId = uint32_t;

class IStorage {
public:
    virtual ~IStorage() = default;
    virtual ComponentId id() const = 0;
    virtual bool has(Entity) const = 0;
    virtual void remove(Entity) = 0;
    virtual size_t size() const = 0;
    virtual void clear() = 0;
    virtual std::span<const Entity> entities() const = 0;
    virtual void* getRaw(Entity) = 0;
    virtual const void* getRaw(Entity) const = 0;
    virtual void* emplaceRaw(Entity, const void* src) = 0;
};

}
