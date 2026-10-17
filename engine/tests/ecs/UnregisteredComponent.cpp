/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** UnregisteredComponent
*/

#include "TestWorld.hpp"

int main() {
    const engine::TypeRegistry types = world::makeTypes();
    engine::Registry registry(types);
    registry.emplace<world::Unregistered>(registry.create());
    return 0;
}
