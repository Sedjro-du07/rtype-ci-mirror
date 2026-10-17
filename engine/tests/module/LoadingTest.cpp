/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** LoadingTest
*/

#include "engine/events/EventBus.hpp"
#include "engine/module/DynamicLibrary.hpp"
#include "engine/module/Export.hpp"
#include "engine/module/ModuleManager.hpp"
#include "os/Os.hpp"

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

namespace engine {

class CVars {};
struct EngineConfig {};

}

using namespace engine;

namespace {

class LineSink final : public ILogSink {
public:
    std::vector<std::string> lines;
    void write(const LogRecord& record) override { this->lines.emplace_back(record.message); }
};

struct Loader {
    std::shared_ptr<LineSink> sink = std::make_shared<LineSink>();
    Logger log{[] {
        LogConfig config;
        config.file.clear();
        config.color = false;
        config.level = LogLevel::Error;
        return config;
    }()};
    CVars cvars;
    EngineConfig config;
    EventBus events;
    ModuleManager modules{log, os::executableDir()};

    Loader() {
        this->log.addSink(this->sink);
        this->log.setLevel(LogLevel::Info);
    }

    PluginContext context() { return PluginContext{this->log, this->cvars, this->config, this->events}; }
};

}

TEST_CASE("a plugin is loaded by kind and name, checked and initialised", "[module][loading]") {
    Loader l;
    const auto plugin = l.modules.loadPlugin<IPlugin>(PluginKind::Network, "loadable", l.context());
    REQUIRE(plugin);
    CHECK((*plugin)->name() == "fixture");
    REQUIRE(l.modules.loaded().size() == 1);
    CHECK(l.modules.loaded()[0].stem == "network_loadable");
    CHECK_FALSE(l.modules.loaded()[0].isGame);
#if !defined(ENGINE_STATIC_MODULES)
    CHECK(l.modules.loaded()[0].file.filename() == DynamicLibrary::decorate("network_loadable"));
#endif
}

TEST_CASE("unknown, mislabeled and null plugins are refused", "[module][loading]") {
    Loader l;
    const auto missing = l.modules.loadPlugin<IPlugin>(PluginKind::Platform, "nothing", l.context());
    REQUIRE_FALSE(missing);
    CHECK(missing.error().code == Errc::PluginNotFound);

    const auto mislabeled = l.modules.loadPlugin<IPlugin>(PluginKind::Audio, "mislabeled", l.context());
    REQUIRE_FALSE(mislabeled);
    CHECK(mislabeled.error().message == "plugin 'audio_mislabeled' is a network plugin, expected audio");

    const auto null = l.modules.loadPlugin<IPlugin>(PluginKind::Audio, "null", l.context());
    REQUIRE_FALSE(null);
    CHECK(null.error().code == Errc::PluginNotFound);
    CHECK(l.modules.loaded().empty());
}

TEST_CASE("one game module at a time, unloaded in reverse order", "[module][loading]") {
    Loader l;
    REQUIRE(l.modules.loadPlugin<IPlugin>(PluginKind::Network, "loadable", l.context()));
    const auto game = l.modules.loadGame("loadable", Role::Client);
    REQUIRE(game);
    CHECK((*game)->firstScene() == "scenes/fixture.scene.yaml");
    CHECK(l.modules.loaded().back().isGame);

    const auto second = l.modules.loadGame("loadable", Role::Client);
    REQUIRE_FALSE(second);
    CHECK(second.error().code == Errc::GameModuleInvalid);
    CHECK_FALSE(l.modules.loadGame("nothing", Role::Client));

    l.sink->lines.clear();
    l.modules.unloadAll();
    CHECK(l.sink->lines == std::vector<std::string>{"unloading game loadable", "unloading plugin network_loadable"});
    CHECK(l.modules.loaded().empty());

    REQUIRE(l.modules.loadGame("loadable", Role::Server));
    l.modules.unloadGame();
    CHECK(l.modules.loaded().empty());
}

#if !defined(ENGINE_STATIC_MODULES)

TEST_CASE("DynamicLibrary opens files and finds symbols", "[module][loading]") {
    const auto dir = os::executableDir();
    const auto missing = DynamicLibrary::open(dir / "plugins" / DynamicLibrary::decorate("platform_nothing"));
    REQUIRE_FALSE(missing);
    CHECK(missing.error().code == Errc::FileNotFound);

    auto library = DynamicLibrary::open(dir / "plugins" / DynamicLibrary::decorate("network_loadable"));
    REQUIRE(library);
    CHECK(library->rawSymbol("create_plugin") != nullptr);
    CHECK(library->rawSymbol("nothing_here") == nullptr);
    using Version = uint32_t (*)() noexcept;
    const auto version = library->symbol<Version>("plugin_api_version");
    REQUIRE(version);
    CHECK((*version)() == ENGINE_PLUGIN_API_VERSION);
    const auto absent = library->symbol<Version>("nothing_here");
    REQUIRE_FALSE(absent);
    CHECK(absent.error().code == Errc::SymbolNotFound);

    DynamicLibrary moved = std::move(*library);
    CHECK(moved.rawSymbol("create_plugin") != nullptr);
    CHECK(moved.path().filename() == DynamicLibrary::decorate("network_loadable"));
}

TEST_CASE("a plugin built for another API version is refused", "[module][loading]") {
    Loader l;
    const auto old = l.modules.loadPlugin<IPlugin>(PluginKind::Network, "oldapi", l.context());
    REQUIRE_FALSE(old);
    CHECK(old.error().code == Errc::ApiMismatch);
    CHECK(old.error().message == "plugin 'network_oldapi' was built for plugin API 2, the engine uses 1");
}

TEST_CASE("a split game loads the build of its role first", "[module][loading]") {
    Loader l;
    auto server = l.modules.loadGame("split", Role::Server);
    REQUIRE(server);
    CHECK((*server)->firstScene() == "server");
    l.modules.unloadGame();
    auto client = l.modules.loadGame("split", Role::Client);
    REQUIRE(client);
    CHECK((*client)->firstScene() == "client");
    l.modules.unloadGame();
    auto editor = l.modules.loadGame("split", Role::Editor);
    REQUIRE(editor);
    CHECK((*editor)->firstScene() == "client");
}

#endif
