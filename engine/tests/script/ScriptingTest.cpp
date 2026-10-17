/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ScriptingTest
*/

#include "engine/script/IScripting.hpp"

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <string>
#include <type_traits>

using namespace engine;

namespace {

class FakeScripting final : public IScripting {
public:
    std::set<uint32_t> loaded;

    PluginKind kind() const override { return PluginKind::Scripting; }
    std::string_view name() const override { return "fake"; }
    uint32_t apiVersion() const override { return 1; }
    Result<void> init(const PluginContext&) override { return {}; }
    void shutdown() noexcept override {}
    Result<void> bind(ScriptHost&) override { return {}; }
    Result<ScriptHandle> load(const std::filesystem::path& file) override {
        if (file.extension() != ".lua") return fail(Errc::ScriptError, "not a script", file.string());
        const ScriptHandle handle{static_cast<uint32_t>(this->loaded.size() + 1)};
        this->loaded.insert(handle.id);
        return handle;
    }
    void unload(ScriptHandle script) override { this->loaded.erase(script.id); }
    Result<void> reload(ScriptHandle) override { return {}; }
    Result<void> call(ScriptHandle, std::string_view, Entity, Context&) override { return {}; }
    bool has(ScriptHandle script, std::string_view function) const override {
        return this->loaded.contains(script.id) && function == "on_update";
    }
};

}

TEST_CASE("a scripting plugin loads, answers and unloads scripts", "[script]") {
    static_assert(std::is_abstract_v<IScripting>);
    FakeScripting fake;
    IScripting& scripting = fake;
    CHECK(scripting.kind() == PluginKind::Scripting);

    const Result<ScriptHandle> boss = scripting.load("scripts/boss.lua");
    REQUIRE(boss.has_value());
    CHECK(scripting.has(*boss, "on_update"));
    CHECK_FALSE(scripting.has(*boss, "on_collision"));

    const Result<ScriptHandle> wrong = scripting.load("scripts/boss.txt");
    REQUIRE_FALSE(wrong.has_value());
    CHECK(wrong.error().code == Errc::ScriptError);

    scripting.unload(*boss);
    CHECK_FALSE(scripting.has(*boss, "on_update"));
}
