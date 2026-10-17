/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** main
*/

#include "engine/Engine.hpp"
#include "engine/EngineConfig.hpp"

#include <cstdio>
#include <utility>

#if !defined(ENGINE_DEFAULT_CONFIG)
#define ENGINE_DEFAULT_CONFIG ""
#endif

int main(int argc, char** argv) {
    auto config = engine::EngineConfig::load(ENGINE_DEFAULT_CONFIG, argc, argv);
    if (!config) {
        std::fprintf(stderr, "%s\n", config.error().toString().c_str());
        return 1;
    }
    engine::Engine engine(std::move(*config));
    return engine.run();
}
