/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EngineTypesTest
*/

#include "engine/components/Animation.hpp"
#include "engine/components/Identity.hpp"
#include "engine/components/Position.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/core/Hash.hpp"
#include "engine/core/math/Rect.hpp"
#include "engine/reflection/TypeRegistry.hpp"
#include "engine/reflection/Yaml.hpp"
#include "systems/RegisterEngineTypes.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

using namespace engine;

namespace {

struct Health {
    int32_t hp = 0;
};

std::string fieldsOf(const TypeInfo& type) {
    std::string text;
    for (const FieldInfo& field : type.fields)
        text += (text.empty() ? "" : " ") + field.name;
    return text;
}

template <class T>
Result<void> read(const TypeRegistry& types, T& value, std::string_view text) {
    const auto node = YamlNode::parse(text, "test.yaml");
    REQUIRE(node);
    LoadContext ctx{types, nullptr, {}};
    return readYaml(types.info(types.id<T>()), &value, *node, ctx);
}

}

ENGINE_COMPONENT(Health)

TEST_CASE("every engine component is registered with its fields and flags", "[components]") {
    TypeRegistry types;
    registerEngineTypes(types);
    const std::vector<std::pair<std::string, std::string>> expected{
        {"Name", "value"},
        {"Uuid", ""},
        {"Tags", ""},
        {"Parent", "value offset"},
        {"Children", "list"},
        {"Position", "x y"},
        {"PreviousPosition", "x y"},
        {"Velocity", "dx dy"},
        {"Sprite", "texture frame layer tint scale origin rotation visible"},
        {"Animation", "frames fps loop time current"},
        {"Text", "value font size color layer screenSpace"},
        {"Collider", "size offset layer mask trigger"},
        {"Lifetime", "seconds"},
        {"Parallax", "factor wrapWidth"},
        {"Script", "file"},
    };
    for (const auto& [name, fields] : expected) {
        const TypeInfo* type = types.find(name);
        REQUIRE(type);
        CHECK(type->category == TypeCategory::Component);
        CHECK(fieldsOf(*type) == fields);
    }
    CHECK((types.find("Position")->flags & Replicated) != 0);
    CHECK((types.find("Sprite")->flags & Replicated) != 0);
    CHECK((types.find("PreviousPosition")->flags & Transient) != 0);
    CHECK((types.find("Children")->flags & Transient) != 0);
    CHECK(types.find("Collision")->category == TypeCategory::Event);
    CHECK(fieldsOf(*types.find("Caption")) == "text seconds");
    CHECK(types.find("Animation")->fields[0].kind == FieldKind::List);
    CHECK(types.find("Animation")->fields[0].element == FieldKind::Rect);
}

TEST_CASE("engine names are reserved for the game", "[components]") {
    TypeRegistry types;
    registerEngineTypes(types);
    types.beginGameTypes();
    types.add<Health>().field("hp", &Health::hp);
    CHECK(types.find("Health"));
    CHECK_THROWS_AS(registerEngineTypes(types), BootError);
}

TEST_CASE("uuids, tags and animations have a YAML form", "[components]") {
    TypeRegistry types;
    registerEngineTypes(types);
    Uuid uuid;
    REQUIRE(read(types, uuid, "a1f3c2d4e5f60718"));
    CHECK(uuid == Uuid{0, 0xa1f3c2d4e5f60718});
    CHECK_FALSE(read(types, uuid, "{ hi: 1 }"));

    Tags tags;
    REQUIRE(read(types, tags, "[player, flying]"));
    CHECK(tags.has("player"));
    CHECK(tags.has("flying"));
    CHECK_FALSE(tags.has("enemy"));
    YamlWriter out;
    SaveContext save{types, nullptr, {}};
    writeYaml(types.info(types.id<Tags>()), &tags, out, save);
    Tags back;
    REQUIRE(read(types, back, out.str()));
    CHECK(back.ids == tags.ids);

    Animation animation;
    REQUIRE(read(types, animation, "{ frames: [[0, 0, 33, 17], { x: 33, w: 33, h: 17 }], fps: 12 }"));
    REQUIRE(animation.frames.size() == 2);
    CHECK(animation.frames[1].x == 33);
    CHECK(animation.fps == 12);

    Sprite sprite;
    REQUIRE(read(types, sprite, "{ layer: 30, tint: '#ff000080', scale: [2, 2] }"));
    CHECK(sprite.layer == 30);
    CHECK(sprite.tint.a == 128);
    CHECK(sprite.scale == Vec2{2, 2});
    CHECK(sprite.visible);
}
