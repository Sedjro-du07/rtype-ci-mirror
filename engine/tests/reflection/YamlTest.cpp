/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** YamlTest
*/

#include "engine/reflection/Yaml.hpp"

#include "scene/SceneFixture.hpp"

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>

using namespace scenetest;

namespace {

Result<void> load(SceneFixture& f, Everything& out, std::string_view text) {
    const auto node = YamlNode::parse(text, "test.yaml");
    REQUIRE(node);
    LoadContext ctx{f.types, &f.assets, {}};
    return readYaml(f.types.info(f.types.id<Everything>()), &out, *node, ctx);
}

std::string errorOf(SceneFixture& f, std::string_view text) {
    Everything value;
    const auto result = load(f, value, text);
    REQUIRE_FALSE(result);
    return result.error().toString();
}

}

TEST_CASE("every field kind survives a round trip", "[yaml]") {
    SceneFixture f("rtype-yaml-roundtrip");
    const Entity ship{7, 1};
    const Uuid shipId{0, 0xa1f3c2d4e5f60718};

    Everything in;
    in.flag = true;
    in.i8 = -128;
    in.u8 = 255;
    in.i16 = -300;
    in.u16 = 65535;
    in.i32 = -70000;
    in.u32 = 4000000000u;
    in.i64 = -9000000000;
    in.f = 0.1f;
    in.d = 2.5;
    in.text = "null";
    in.v = {1.5f, -2.f};
    in.r = {1, 2, 33, 17};
    in.c = {255, 128, 0, 64};
    in.target = ship;
    in.texture = f.assets.texture("assets/ship.png");
    in.kind = Kind::Boss;
    in.waves = {1, 2, 3};
    in.stats = {{5, "fast"}, {9, "slow"}};
    in.kinds = {Kind::Boss, Kind::Grunt};
    in.main = {42, "main gun"};

    YamlWriter out;
    SaveContext save{f.types, &f.assets, [&](Entity e) -> std::optional<Uuid> {
                         if (e == ship) return shipId;
                         return std::nullopt;
                     }};
    writeYaml(f.types.info(f.types.id<Everything>()), &in, out, save);
    const std::string text = out.str();
    CHECK(text.find("kind: Boss") != std::string::npos);
    CHECK(text.find("target: a1f3c2d4e5f60718") != std::string::npos);
    CHECK(text.find("texture: assets/ship.png") != std::string::npos);

    const auto node = YamlNode::parse(text, "saved.yaml");
    REQUIRE(node);
    LoadContext ctx{f.types, &f.assets, [&](const Uuid& id) { return id == shipId ? ship : Entity::null(); }};
    Everything back;
    REQUIRE(readYaml(f.types.info(f.types.id<Everything>()), &back, *node, ctx));
    CHECK(back.flag);
    CHECK(back.i8 == -128);
    CHECK(back.u8 == 255);
    CHECK(back.i16 == -300);
    CHECK(back.u16 == 65535);
    CHECK(back.i32 == -70000);
    CHECK(back.u32 == 4000000000u);
    CHECK(back.i64 == -9000000000);
    CHECK(back.f == 0.1f);
    CHECK(back.d == 2.5);
    CHECK(back.text == "null");
    CHECK(back.v == in.v);
    CHECK(back.r.w == 33);
    CHECK(back.r.h == 17);
    CHECK(back.c == in.c);
    CHECK(back.target == ship);
    CHECK(back.texture == in.texture);
    CHECK(back.kind == Kind::Boss);
    CHECK(back.waves == in.waves);
    REQUIRE(back.stats.size() == 2);
    CHECK(back.stats[1].power == 9);
    CHECK(back.stats[1].label == "slow");
    CHECK(back.kinds == in.kinds);
    CHECK(back.main.power == 42);
    CHECK(back.main.label == "main gun");
}

TEST_CASE("missing fields keep their value and short forms are accepted", "[yaml]") {
    SceneFixture f("rtype-yaml-forms");
    Everything value;
    value.i32 = 12;
    REQUIRE(load(f, value, "{ v: [3, 4], r: { w: 10 }, c: '#ff0000' }"));
    CHECK(value.i32 == 12);
    CHECK(value.v == Vec2{3, 4});
    CHECK(value.r.w == 10);
    CHECK(value.c == Color{255, 0, 0, 255});
    REQUIRE(load(f, value, "{ c: [1, 2, 3, 4], v: { y: 9 } }"));
    CHECK(value.c == Color{1, 2, 3, 4});
    CHECK(value.v == Vec2{3, 9});
    REQUIRE(load(f, value, ""));
}

TEST_CASE("errors name the file, the line and a close name", "[yaml]") {
    SceneFixture f("rtype-yaml-errors");
    CHECK(errorOf(f, "flag: true\ni8: 300\n") ==
          "test.yaml:2: field 'i8' of Everything expects an integer from -128 to 127, got \"300\"");
    CHECK(errorOf(f, "i32: three") ==
          "test.yaml:1: field 'i32' of Everything expects an integer from -2147483648 to 2147483647, got \"three\"");
    CHECK(errorOf(f, "flg: true") == "test.yaml:1: unknown field 'flg' in Everything (did you mean 'flag'?)");
    CHECK(errorOf(f, "kind: Bos") ==
          "test.yaml:1: field 'kind' of Everything has no value 'Bos' (did you mean 'Boss'?)");
    CHECK(errorOf(f, "u8: -1").find("expects an integer from 0 to 255") != std::string::npos);
    CHECK(errorOf(f, "waves: [1, 2, 3, 4, 5]") ==
          "test.yaml:1: field 'waves' of Everything holds at most 4 elements, got 5");
    CHECK(errorOf(f, "main: { powr: 1 }") == "test.yaml:1: unknown field 'powr' in Stats (did you mean 'power'?)");
    CHECK(errorOf(f, "v: { x: 1, z: 2 }").find("unknown key 'z'") != std::string::npos);
    CHECK(errorOf(f, "target: not-hex").find("expects an entity uuid") != std::string::npos);
    CHECK(errorOf(f, "[1, 2]") == "test.yaml:1: Everything expects a map of fields, got a list");
}

TEST_CASE("a broken file reports the line of the syntax error", "[yaml]") {
    const auto node = YamlNode::parse("a: 1\nb: [2, 3\n", "broken.yaml");
    REQUIRE_FALSE(node);
    CHECK(node.error().code == Errc::ParseError);
    CHECK(node.error().file == "broken.yaml");
    CHECK(node.error().line >= 2);
}

TEST_CASE("uuids are written in hexadecimal and read back", "[yaml]") {
    CHECK(formatUuid(Uuid{0, 0xb05}) == "0000000000000b05");
    CHECK(formatUuid(Uuid{1, 2}) == "00000000000000010000000000000002");
    CHECK(parseUuid("0000000000000b05") == Uuid{0, 0xb05});
    CHECK(parseUuid("00000000-00000001-00000000-00000002") == Uuid{1, 2});
    CHECK(parseUuid("ABC") == Uuid{0, 0xabc});
    CHECK_FALSE(parseUuid(""));
    CHECK_FALSE(parseUuid("xyz"));
    CHECK_FALSE(parseUuid(std::string(33, 'f')));
}
