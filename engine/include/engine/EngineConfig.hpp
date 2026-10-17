/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EngineConfig
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Error.hpp"
#include "engine/net/NetTypes.hpp"
#include "engine/render/WindowDesc.hpp"
#include "engine/services/Logger.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace engine {

enum class Role : uint8_t { Client, Server, Editor };

ENGINE_API std::string_view roleName(Role role);
ENGINE_API std::optional<Role> roleFromName(std::string_view name);

struct ENGINE_API EngineConfig {
    Role role = Role::Client;
    std::string game;
    std::string platform = "sfml";
    std::string audio = "miniaudio";
    std::string network = "asio";
    std::string scripting = "lua";
    double tickRate = 60.0;
    std::filesystem::path root;
    std::string host = "127.0.0.1";
    WindowDesc window;
    NetConfig net;
    LogConfig log;
    std::filesystem::path sceneOverride;
    uint64_t maxTicks = 0;

    static Result<EngineConfig> load(const std::filesystem::path& file, int argc, char** argv);
    static Result<EngineConfig> load(const std::filesystem::path& file, std::span<const std::string_view> args);
};

}
