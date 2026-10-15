/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TypeRegistryTest
*/

#include "engine/reflection/TypeRegistry.hpp"

#include "engine/core/Error.hpp"
#include "engine/ecs/SparseSet.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace game {

struct Health {
    int32_t hp = 100;
    int32_t maxHp = 100;
};

struct Position {
    engine::Vec2 value;
};

enum class EnemyKind { Grunt, Boss = 5 };

struct Spawn {
    EnemyKind kind = EnemyKind::Grunt;
    engine::Entity target;
    std::vector<int32_t> waves;
};

struct BossDefeated {
    int32_t score = 0;
};

struct RoomInfo {
    std::string name;
    uint32_t players = 0;
};

struct RoomList {
    std::vector<RoomInfo> rooms;
};

struct Chat {
    std::string text;
};

struct Move {
    float x = 0;
};

}

ENGINE_COMPONENT(game::Health)
ENGINE_COMPONENT(game::Position)
ENGINE_COMPONENT(game::Spawn)
ENGINE_ENUM(game::EnemyKind, Grunt, Boss)
ENGINE_EVENT(game::BossDefeated)
ENGINE_STRUCT(game::RoomInfo)
ENGINE_MESSAGE(game::RoomList)
ENGINE_MESSAGE(game::Chat)
ENGINE_MESSAGE(game::Move)

using namespace engine;

TEST_CASE("a component registers with its fields and functions", "[reflection]") {
    TypeRegistry types;
    types.add<game::Health>().field("hp", &game::Health::hp).field("maxHp", &game::Health::maxHp).flags(Replicated);

    const TypeInfo* info = types.find("Health");
    REQUIRE(info != nullptr);
    CHECK(info->name == "Health");
    CHECK(info->key == fnv1a64("Health"));
    CHECK(info->category == TypeCategory::Component);
    CHECK(info->size == sizeof(game::Health));
    CHECK(info->flags == Replicated);
    REQUIRE(info->fields.size() == 2);
    CHECK(info->fields[0].name == "hp");
    CHECK(info->fields[0].kind == FieldKind::Int32);
    CHECK(info->fields[0].offset == offsetof(game::Health, hp));
    CHECK(info->fields[1].offset == offsetof(game::Health, maxHp));
    CHECK(types.id<game::Health>() == info->id);
    CHECK(&types.info(info->id) == info);
    CHECK(types.findByKey(typeKey<game::Health>()) == info);

    std::unique_ptr<IStorage> storage = info->makeStorage(info->id);
    CHECK(storage->id() == info->id);
    const game::Health source{40, 80};
    storage->emplaceRaw(Entity{0, 0}, &source);
    CHECK(static_cast<SparseSet<game::Health>&>(*storage).get(Entity{0, 0}).hp == 40);

    alignas(game::Health) std::byte buffer[sizeof(game::Health)];
    info->construct(buffer);
    CHECK(reinterpret_cast<game::Health*>(buffer)->hp == 100);
    info->copy(buffer, &source);
    CHECK(reinterpret_cast<game::Health*>(buffer)->maxHp == 80);
    info->destroy(buffer);
}

TEST_CASE("field kinds are deduced from member types", "[reflection]") {
    TypeRegistry types;
    types.add<game::EnemyKind>();
    types.add<game::Position>().field("value", &game::Position::value);
    types.add<game::Spawn>()
        .field("kind", &game::Spawn::kind)
        .field("target", &game::Spawn::target)
        .field("waves", &game::Spawn::waves)
        .maxLength(8);

    CHECK(types.find("Position")->fields[0].kind == FieldKind::Vec2);
    const auto& fields = types.find("Spawn")->fields;
    CHECK(fields[0].kind == FieldKind::Enum);
    REQUIRE(fields[0].enumInfo != nullptr);
    REQUIRE(fields[0].enumInfo->values.size() == 2);
    CHECK(fields[0].enumInfo->values[0].first == "Grunt");
    CHECK(fields[0].enumInfo->values[1].first == "Boss");
    CHECK(fields[0].enumInfo->values[1].second == 5);
    CHECK(fields[1].kind == FieldKind::Entity);
    CHECK(fields[2].kind == FieldKind::List);
    CHECK(fields[2].element == FieldKind::Int32);
    CHECK(fields[2].maxLength == 8);
    CHECK(types.find("EnemyKind")->category == TypeCategory::Enum);
}

TEST_CASE("events, structs and messages register with their options", "[reflection]") {
    TypeRegistry types;
    types.add<game::BossDefeated>().field("score", &game::BossDefeated::score);
    types.add<game::RoomInfo>()
        .field("name", &game::RoomInfo::name)
        .maxLength(32)
        .field("players", &game::RoomInfo::players);
    types.add<game::RoomList>().field("rooms", &game::RoomList::rooms).maxLength(32).stream();
    types.add<game::Chat>().field("text", &game::Chat::text).reliable();
    types.add<game::Move>().field("x", &game::Move::x).quantize(-1.f, 1.f).unreliable();

    CHECK(types.find("BossDefeated")->category == TypeCategory::Event);
    CHECK(types.find("RoomInfo")->category == TypeCategory::Struct);
    CHECK(types.find("RoomInfo")->fields[0].maxLength == 32);

    const TypeInfo* list = types.find("RoomList");
    CHECK(list->transport == Transport::Stream);
    CHECK(list->fields[0].kind == FieldKind::List);
    CHECK(list->fields[0].element == FieldKind::Struct);
    CHECK(list->fields[0].elementType == typeKey<game::RoomInfo>());
    CHECK(list->makeStorage == nullptr);

    const TypeInfo* move = types.find("Move");
    CHECK(move->transport == Transport::Unreliable);
    CHECK(move->fields[0].quantMin == -1.f);
    CHECK(move->fields[0].quantMax == 1.f);

    const auto messages = types.messages();
    REQUIRE(messages.size() == 3);
    CHECK(messages[0]->name == "RoomList");
    CHECK(messages[1]->name == "Chat");
    CHECK(messages[2]->name == "Move");
    CHECK(types.all().size() == 5);
}

TEST_CASE("registering a name twice is a DuplicateName boot error", "[reflection]") {
    TypeRegistry types;
    types.add<game::Health>();
    try {
        types.add<game::Health>();
        FAIL("no BootError thrown");
    } catch (const BootError& error) {
        CHECK(error.exitCode() == 3);
        CHECK(error.error().code == Errc::DuplicateName);
    }
    CHECK(types.all().size() == 1);
}

TEST_CASE("the fingerprint follows the wire format only", "[reflection]") {
    TypeRegistry a;
    a.add<game::Health>().field("hp", &game::Health::hp).flags(Replicated);
    a.add<game::Chat>().field("text", &game::Chat::text).maxLength(64);

    TypeRegistry same;
    same.add<game::Health>().field("hp", &game::Health::hp).flags(Replicated);
    same.add<game::Chat>().field("text", &game::Chat::text).maxLength(64);
    CHECK(a.formatFingerprint() == same.formatFingerprint());

    TypeRegistry otherField;
    otherField.add<game::Health>().field("maxHp", &game::Health::maxHp).flags(Replicated);
    otherField.add<game::Chat>().field("text", &game::Chat::text).maxLength(64);
    CHECK(a.formatFingerprint() != otherField.formatFingerprint());

    TypeRegistry otherLimit;
    otherLimit.add<game::Health>().field("hp", &game::Health::hp).flags(Replicated);
    otherLimit.add<game::Chat>().field("text", &game::Chat::text).maxLength(32);
    CHECK(a.formatFingerprint() != otherLimit.formatFingerprint());

    TypeRegistry localOnly;
    localOnly.add<game::Health>().field("hp", &game::Health::hp).flags(Replicated);
    localOnly.add<game::Chat>().field("text", &game::Chat::text).maxLength(64);
    localOnly.add<game::Position>().field("value", &game::Position::value);
    CHECK(a.formatFingerprint() == localOnly.formatFingerprint());
}

TEST_CASE("closestName suggests a registered name", "[reflection]") {
    TypeRegistry types;
    types.add<game::Health>();
    types.add<game::Position>();
    CHECK(types.closestName("Helth") == "Health");
    CHECK(types.closestName("position") == "Position");
    CHECK(types.closestName("Velocity") == "");
}

TEST_CASE("clearGameTypes keeps the engine types", "[reflection]") {
    TypeRegistry types;
    types.add<game::Position>();
    types.beginGameTypes();
    types.add<game::Health>();
    types.add<game::Chat>();

    types.clearGameTypes();
    CHECK(types.all().size() == 1);
    CHECK(types.find("Position") != nullptr);
    CHECK(types.find("Health") == nullptr);
    CHECK(types.messages().empty());

    types.add<game::Health>();
    CHECK(types.find("Health")->id == 1);
}
