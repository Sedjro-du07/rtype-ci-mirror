/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** CoreTypesTest
*/

#include "engine/core/Error.hpp"
#include "engine/core/Hash.hpp"
#include "engine/core/math/Color.hpp"
#include "engine/core/math/Rect.hpp"
#include "engine/core/math/Vec2.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <string>

using namespace engine;

static_assert(fnv1a64("") == 0xcbf29ce484222325ull);
static_assert(fnv1a64("a") == 0xaf63dc4c8601ec8cull);
static_assert(fnv1a64("foobar") == 0x85944171f73967e8ull);

TEST_CASE("fnv1a64 distinguishes close names", "[core][hash]") {
    CHECK(fnv1a64("Position") != fnv1a64("position"));
    CHECK(fnv1a64("Health") != fnv1a64("Helth"));
}

TEST_CASE("Vec2 arithmetic", "[core][math]") {
    const Vec2 a{3, 4};
    CHECK(a + Vec2{1, 1} == Vec2{4, 5});
    CHECK(a - Vec2{1, 1} == Vec2{2, 3});
    CHECK(a * 2.f == Vec2{6, 8});
    CHECK(a / 2.f == Vec2{1.5f, 2});
    CHECK(a.dot({1, 0}) == 3.f);
    CHECK(Vec2::lerp({0, 0}, {10, 20}, 0.5f) == Vec2{5, 10});

    CHECK(a.length() == Catch::Approx(5.f));
    CHECK(a.normalized().length() == Catch::Approx(1.f));
    CHECK(Vec2{}.normalized() == Vec2{});

    Vec2 b{1, 2};
    b += {1, 1};
    b *= 2.f;
    b -= {1, 1};
    CHECK(b == Vec2{3, 5});
}

TEST_CASE("Rect edges", "[core][math]") {
    const Rect r{0, 0, 10, 10};
    CHECK(r.contains({0, 0}));
    CHECK(r.contains({9.9f, 9.9f}));
    CHECK(!r.contains({10, 5}));
    CHECK(!r.contains({5, 10}));
    CHECK(r.intersects({5, 5, 10, 10}));
    CHECK(!r.intersects({10, 0, 5, 5}));
    CHECK(!r.intersects({0, 10, 5, 5}));
    CHECK(r.center() == Vec2{5, 5});
}

TEST_CASE("Color constants", "[core][math]") {
    CHECK(Color{} == Color::white());
    CHECK(Color::black() == Color{0, 0, 0, 255});
    CHECK(Color::transparent().a == 0);
    CHECK(Color::red() != Color::blue());
}

TEST_CASE("Error::toString with and without a file", "[core][error]") {
    CHECK(Error{Errc::ConfigInvalid, "missing key 'game'"}.toString() == "missing key 'game'");
    CHECK(Error{Errc::ParseError, "unknown component 'Helth'", "scenes/level1.yaml", 14}.toString() ==
          "scenes/level1.yaml:14: unknown component 'Helth'");
    CHECK(Error{Errc::FileNotFound, "cannot open", "client.yaml"}.toString() == "client.yaml: cannot open");
}

namespace {
Result<int> half(int value) {
    if (value % 2 != 0) return fail(Errc::InvalidArgument, "odd value");
    return value / 2;
}
}

TEST_CASE("Result carries a value or an Error", "[core][error]") {
    const auto even = half(4);
    REQUIRE(even.has_value());
    CHECK(*even == 2);

    const auto odd = half(3);
    REQUIRE_FALSE(odd.has_value());
    CHECK(odd.error().code == Errc::InvalidArgument);
    CHECK(odd.error().message == "odd value");
}

TEST_CASE("BootError keeps the exit code and the error", "[core][error]") {
    const BootError e(2, Error{Errc::PluginNotFound, "plugin 'platform_xyz' not found"});
    CHECK(e.exitCode() == 2);
    CHECK(e.error().code == Errc::PluginNotFound);
    CHECK(std::string(e.what()) == "plugin 'platform_xyz' not found");
}
