/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ThrowingModuleTest
*/

#include "engine/module/Export.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace engine;

#if defined(ENGINE_STATIC_MODULES)

TEST_CASE("a game whose constructor throws gives no module", "[module]") {
    const GameEntry* entry = StaticModuleTable::findGame("game_fixture_throw");
    REQUIRE(entry != nullptr);
    CHECK(entry->create() == nullptr);
}

#else

extern "C" IGameModule* create_game() noexcept;

TEST_CASE("a game whose constructor throws gives no module", "[module]") {
    CHECK(create_game() == nullptr);
}

#endif
