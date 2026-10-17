/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EngineConfigTest
*/

#include "engine/EngineConfig.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

using namespace engine;

namespace {

struct ConfigFile {
    std::filesystem::path folder;
    std::filesystem::path path;

    ConfigFile(const std::string& name, const std::string& content)
        : folder(std::filesystem::temp_directory_path() / ("rtype-config-" + name)), path(folder / "rtype.yaml") {
        std::filesystem::create_directories(this->folder);
        std::ofstream(this->path, std::ios::binary) << content;
    }
    ~ConfigFile() { std::filesystem::remove_all(this->folder); }
};

const std::string Valid = "role: client\n"
                          "game: rtype\n"
                          "platform: sfml\n"
                          "audio: miniaudio\n"
                          "tickRate: 60\n"
                          "root: game\n"
                          "host: 10.0.0.5\n"
                          "window: { title: R-Type, width: 1600, height: 900 }\n"
                          "net: { port: 4243, timeoutSeconds: 8 }\n"
                          "log: { level: warn, file: logs/client.log }\n";

}

TEST_CASE("a configuration file fills every section", "[config]") {
    ConfigFile file("1", Valid);
    const auto config = EngineConfig::load(file.path, std::vector<std::string_view>{});
    REQUIRE(config);
    CHECK(config->role == Role::Client);
    CHECK(config->game == "rtype");
    CHECK(config->tickRate == 60.0);
    CHECK(config->root == file.folder / "game");
    CHECK(config->host == "10.0.0.5");
    CHECK(config->window.title == "R-Type");
    CHECK(config->window.width == 1600);
    CHECK(config->window.virtualWidth == 1280);
    CHECK(config->net.port == 4243);
    CHECK(config->net.timeoutSeconds == 8.0);
    CHECK(config->log.level == LogLevel::Warn);
    CHECK(config->log.file == "logs/client.log");
    CHECK(config->network == "asio");
}

TEST_CASE("the command line overrides the file", "[config]") {
    ConfigFile file("2", Valid);
    const std::vector<std::string_view> args{"--role",     "server", "--port=5000", "--audio", "null",
                                             "--platform", "null",   "--network",   "null",    "--scripting",
                                             "",           "--host", "example.org", "--scene", "scenes/test.yaml",
                                             "--ticks",    "1000",   "--log",       "debug"};
    const auto config = EngineConfig::load(file.path, args);
    REQUIRE(config);
    CHECK(config->role == Role::Server);
    CHECK(config->net.port == 5000);
    CHECK(config->audio == "null");
    CHECK(config->platform == "null");
    CHECK(config->network == "null");
    CHECK(config->scripting.empty());
    CHECK(config->host == "example.org");
    CHECK(config->sceneOverride == "scenes/test.yaml");
    CHECK(config->maxTicks == 1000);
    CHECK(config->log.level == LogLevel::Debug);
    CHECK(config->game == "rtype");
}

TEST_CASE("configuration errors name the file and the line", "[config]") {
    {
        ConfigFile file("3", "game: rtype\ntickrate: 60\n");
        const auto config = EngineConfig::load(file.path, std::vector<std::string_view>{});
        REQUIRE_FALSE(config);
        CHECK(config.error().code == Errc::ConfigInvalid);
        CHECK(config.error().line == 2);
        CHECK(config.error().message == "unknown key 'tickrate' in the configuration (did you mean 'tickRate'?)");
    }
    {
        ConfigFile file("4", "net: { port: 70000 }\n");
        const auto config = EngineConfig::load(file.path, std::vector<std::string_view>{});
        REQUIRE_FALSE(config);
        CHECK(config.error().message.starts_with("'port' expects an integer from 0 to 65535"));
    }
    {
        ConfigFile file("5", "role: player\n");
        CHECK_FALSE(EngineConfig::load(file.path, std::vector<std::string_view>{}));
    }
    ConfigFile file("6", Valid);
    CHECK_FALSE(EngineConfig::load(file.path, std::vector<std::string_view>{"--port", "abc"}));
    CHECK_FALSE(EngineConfig::load(file.path, std::vector<std::string_view>{"--colour", "red"}));
    CHECK_FALSE(EngineConfig::load(file.path, std::vector<std::string_view>{"--role"}));
    CHECK_FALSE(EngineConfig::load(file.path, std::vector<std::string_view>{"server"}));
    const auto missing = EngineConfig::load(file.folder / "nope.yaml", std::vector<std::string_view>{});
    REQUIRE_FALSE(missing);
    CHECK(missing.error().code == Errc::FileNotFound);
}

TEST_CASE("the config option picks another file and argv skips the program name", "[config]") {
    ConfigFile file("7", Valid);
    const std::string option = "--config=" + file.path.string();
    std::vector<std::string> storage{"rtype", option, "--role", "server"};
    std::vector<char*> argv;
    for (std::string& arg : storage)
        argv.push_back(arg.data());
    const auto config = EngineConfig::load("missing-default.yaml", static_cast<int>(argv.size()), argv.data());
    REQUIRE(config);
    CHECK(config->role == Role::Server);
    CHECK(config->game == "rtype");
    CHECK(roleName(Role::Editor) == "editor");
    CHECK(roleFromName("server") == Role::Server);
    CHECK_FALSE(roleFromName("Server"));
}
