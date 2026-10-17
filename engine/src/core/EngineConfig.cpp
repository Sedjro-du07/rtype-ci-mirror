/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EngineConfig
*/

#include "engine/EngineConfig.hpp"

#include "engine/reflection/Yaml.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <format>
#include <functional>
#include <limits>
#include <map>
#include <type_traits>
#include <utility>
#include <vector>

namespace engine {

namespace {

using Reader = std::function<Result<void>(const YamlNode&)>;

constexpr std::array<std::string_view, 5> LevelNames{"trace", "debug", "info", "warn", "error"};

unexpected<Error> invalid(const YamlNode& node, std::string message) {
    auto error = node.error(Errc::ConfigInvalid, std::move(message));
    return error;
}

template <class T>
Reader number(T& out, std::string_view key, T low = std::numeric_limits<T>::lowest(),
              T high = std::numeric_limits<T>::max()) {
    return [&out, key, low, high](const YamlNode& node) -> Result<void> {
        if constexpr (std::is_floating_point_v<T>) {
            const auto value = node.asDouble();
            if (!value || *value < low || *value > high)
                return invalid(
                    node, std::format("'{}' expects a number from {} to {}, got {}", key, low, high, node.describe()));
            out = static_cast<T>(*value);
        } else {
            const auto value = node.asUInt();
            if (!value || *value < static_cast<uint64_t>(low) || *value > static_cast<uint64_t>(high))
                return invalid(node, std::format("'{}' expects an integer from {} to {}, got {}", key, low, high,
                                                 node.describe()));
            out = static_cast<T>(*value);
        }
        return {};
    };
}

Reader flag(bool& out, std::string_view key) {
    return [&out, key](const YamlNode& node) -> Result<void> {
        const auto value = node.asBool();
        if (!value) return invalid(node, std::format("'{}' expects true or false, got {}", key, node.describe()));
        out = *value;
        return {};
    };
}

Reader text(std::string& out, std::string_view key) {
    return [&out, key](const YamlNode& node) -> Result<void> {
        if (node.isNull()) {
            out.clear();
            return {};
        }
        if (!node.isScalar()) return invalid(node, std::format("'{}' expects a string, got {}", key, node.describe()));
        out = node.scalar();
        return {};
    };
}

Reader path(std::filesystem::path& out, std::string_view key) {
    return [&out, key](const YamlNode& node) -> Result<void> {
        if (node.isNull()) {
            out.clear();
            return {};
        }
        if (!node.isScalar()) return invalid(node, std::format("'{}' expects a path, got {}", key, node.describe()));
        out = node.scalar();
        return {};
    };
}

std::optional<LogLevel> levelFromName(std::string_view name) {
    const auto it = std::ranges::find(LevelNames, name);
    if (it == LevelNames.end()) return std::nullopt;
    return static_cast<LogLevel>(it - LevelNames.begin());
}

Reader level(LogLevel& out, std::string_view key) {
    return [&out, key](const YamlNode& node) -> Result<void> {
        const auto value = node.isScalar() ? levelFromName(node.scalar()) : std::nullopt;
        if (!value)
            return invalid(node,
                           std::format("'{}' expects trace, debug, info, warn or error, got {}", key, node.describe()));
        out = *value;
        return {};
    };
}

Reader role(Role& out) {
    return [&out](const YamlNode& node) -> Result<void> {
        const auto value = node.isScalar() ? roleFromName(node.scalar()) : std::nullopt;
        if (!value)
            return invalid(node, std::format("'role' expects client, server or editor, got {}", node.describe()));
        out = *value;
        return {};
    };
}

size_t editDistance(std::string_view a, std::string_view b) {
    std::vector<size_t> row(b.size() + 1);
    for (size_t j = 0; j <= b.size(); ++j)
        row[j] = j;
    for (size_t i = 1; i <= a.size(); ++i) {
        size_t diagonal = row[0];
        row[0] = i;
        for (size_t j = 1; j <= b.size(); ++j) {
            const size_t above = row[j];
            row[j] = std::min({row[j] + 1, row[j - 1] + 1, diagonal + (a[i - 1] == b[j - 1] ? 0 : 1)});
            diagonal = above;
        }
    }
    return row[b.size()];
}

Result<void> readMap(const YamlNode& node, std::string_view section, const std::map<std::string, Reader>& readers) {
    if (node.isNull()) return {};
    if (!node.isMap()) return invalid(node, std::format("'{}' expects a map, got {}", section, node.describe()));
    for (const auto& [key, value] : node.entries()) {
        const auto reader = readers.find(key);
        if (reader == readers.end()) {
            std::string best;
            size_t bestDistance = 3;
            for (const auto& [known, unused] : readers)
                if (const size_t d = editDistance(key, known); d < bestDistance) {
                    bestDistance = d;
                    best = known;
                }
            return invalid(value,
                           std::format("unknown key '{}' in {}{}", key, section,
                                       best.empty() ? std::string{} : std::format(" (did you mean '{}'?)", best)));
        }
        if (auto ok = reader->second(value); !ok) return ok;
    }
    return {};
}

Result<void> readFile(EngineConfig& config, const std::filesystem::path& file) {
    auto root = YamlNode::load(file, file.generic_string());
    if (!root) {
        Error error = root.error();
        error.code = error.code == Errc::FileNotFound ? Errc::FileNotFound : Errc::ConfigInvalid;
        return unexpected(std::move(error));
    }
    WindowDesc& w = config.window;
    NetConfig& n = config.net;
    LogConfig& l = config.log;
    const std::map<std::string, Reader> window{
        {"title", text(w.title, "title")},
        {"width", number<uint32_t>(w.width, "width", 1, 16384)},
        {"height", number<uint32_t>(w.height, "height", 1, 16384)},
        {"virtualWidth", number<uint32_t>(w.virtualWidth, "virtualWidth", 1, 16384)},
        {"virtualHeight", number<uint32_t>(w.virtualHeight, "virtualHeight", 1, 16384)},
        {"fullscreen", flag(w.fullscreen, "fullscreen")},
        {"vsync", flag(w.vsync, "vsync")},
        {"uiScale", number<float>(w.uiScale, "uiScale", 0.5f, 4.f)},
    };
    const std::map<std::string, Reader> net{
        {"port", number<uint16_t>(n.port, "port")},
        {"maxClients", number<uint32_t>(n.maxClients, "maxClients", 1, 4096)},
        {"heartbeatSeconds", number<double>(n.heartbeatSeconds, "heartbeatSeconds", 0.01, 60)},
        {"timeoutSeconds", number<double>(n.timeoutSeconds, "timeoutSeconds", 0.1, 600)},
        {"maxDatagramBytes", number<uint32_t>(n.maxDatagramBytes, "maxDatagramBytes", 64, 1200)},
        {"maxQueuedDatagrams", number<uint32_t>(n.maxQueuedDatagrams, "maxQueuedDatagrams", 1, 1000000)},
        {"snapshotRate", number<double>(n.snapshotRate, "snapshotRate", 1, 240)},
        {"maxNameLength", number<uint32_t>(n.maxNameLength, "maxNameLength", 1, 256)},
        {"maxStreamMessageBytes", number<uint32_t>(n.maxStreamMessageBytes, "maxStreamMessageBytes", 16, 65536)},
        {"handshakeSeconds", number<double>(n.handshakeSeconds, "handshakeSeconds", 0.1, 600)},
        {"maxPendingHandshakes", number<uint32_t>(n.maxPendingHandshakes, "maxPendingHandshakes", 1, 4096)},
        {"reconnectGraceSeconds", number<double>(n.reconnectGraceSeconds, "reconnectGraceSeconds", 0, 3600)},
        {"autoReconnect", flag(n.autoReconnect, "autoReconnect")},
        {"sendBudgetBytesPerSecond", number<uint32_t>(n.sendBudgetBytesPerSecond, "sendBudgetBytesPerSecond", 1024)},
        {"interpolationDelay", number<double>(n.interpolationDelay, "interpolationDelay", 0, 2)},
        {"inputRedundancy", number<uint32_t>(n.inputRedundancy, "inputRedundancy", 1, 4)},
        {"shareOnlinePlayers", flag(n.shareOnlinePlayers, "shareOnlinePlayers")},
    };
    const std::map<std::string, Reader> log{
        {"level", level(l.level, "level")},
        {"file", path(l.file, "file")},
        {"color", flag(l.color, "color")},
    };
    const std::map<std::string, Reader> top{
        {"role", role(config.role)},
        {"game", text(config.game, "game")},
        {"platform", text(config.platform, "platform")},
        {"audio", text(config.audio, "audio")},
        {"network", text(config.network, "network")},
        {"scripting", text(config.scripting, "scripting")},
        {"tickRate", number<double>(config.tickRate, "tickRate", 1, 1000)},
        {"root", path(config.root, "root")},
        {"host", text(config.host, "host")},
        {"scene", path(config.sceneOverride, "scene")},
        {"ticks", number<uint64_t>(config.maxTicks, "ticks")},
        {"window", [&](const YamlNode& node) { return readMap(node, "window", window); }},
        {"net", [&](const YamlNode& node) { return readMap(node, "net", net); }},
        {"log", [&](const YamlNode& node) { return readMap(node, "log", log); }},
    };
    if (auto ok = readMap(*root, "the configuration", top); !ok) return ok;
    if (config.root.is_relative()) config.root = file.parent_path() / config.root;
    return {};
}

template <class T>
Result<void> parseInteger(std::string_view option, std::string_view value, T& out) {
    T parsed{};
    const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), parsed);
    if (error != std::errc{} || end != value.data() + value.size())
        return fail(Errc::ConfigInvalid, std::format("option {} expects an integer, got '{}'", option, value));
    out = parsed;
    return {};
}

Result<void> applyOption(EngineConfig& config, std::string_view option, std::string_view value) {
    if (option == "--role") {
        const auto parsed = roleFromName(value);
        if (!parsed)
            return fail(Errc::ConfigInvalid, std::format("--role expects client, server or editor, got '{}'", value));
        config.role = *parsed;
    } else if (option == "--platform") {
        config.platform = std::string(value);
    } else if (option == "--audio") {
        config.audio = std::string(value);
    } else if (option == "--network") {
        config.network = std::string(value);
    } else if (option == "--scripting") {
        config.scripting = std::string(value);
    } else if (option == "--host") {
        config.host = std::string(value);
    } else if (option == "--port") {
        return parseInteger(option, value, config.net.port);
    } else if (option == "--ticks") {
        return parseInteger(option, value, config.maxTicks);
    } else if (option == "--scene") {
        config.sceneOverride = std::filesystem::path(value);
    } else if (option == "--log") {
        const auto parsed = levelFromName(value);
        if (!parsed)
            return fail(Errc::ConfigInvalid,
                        std::format("--log expects trace, debug, info, warn or error, got '{}'", value));
        config.log.level = *parsed;
    } else {
        return fail(Errc::ConfigInvalid, std::format("unknown option '{}'", option));
    }
    return {};
}

}

std::string_view roleName(Role role) {
    switch (role) {
    case Role::Server:
        return "server";
    case Role::Editor:
        return "editor";
    case Role::Client:
        break;
    }
    return "client";
}

std::optional<Role> roleFromName(std::string_view name) {
    if (name == "client") return Role::Client;
    if (name == "server") return Role::Server;
    if (name == "editor") return Role::Editor;
    return std::nullopt;
}

Result<EngineConfig> EngineConfig::load(const std::filesystem::path& file, std::span<const std::string_view> args) {
    std::vector<std::pair<std::string_view, std::string_view>> options;
    for (size_t i = 0; i < args.size(); ++i) {
        std::string_view option = args[i];
        if (!option.starts_with("--"))
            return fail(Errc::ConfigInvalid, std::format("unexpected argument '{}' (options start with --)", option));
        std::string_view value;
        if (const size_t equal = option.find('='); equal != std::string_view::npos) {
            value = option.substr(equal + 1);
            option = option.substr(0, equal);
        } else {
            if (i + 1 >= args.size()) return fail(Errc::ConfigInvalid, std::format("option {} needs a value", option));
            value = args[++i];
        }
        options.emplace_back(option, value);
    }

    std::filesystem::path chosen = file;
    for (const auto& [option, value] : options)
        if (option == "--config") chosen = std::filesystem::path(value);

    EngineConfig config;
    if (!chosen.empty()) {
        std::error_code error;
        if (!std::filesystem::is_regular_file(chosen, error))
            return fail(Errc::FileNotFound, "configuration file not found", chosen.generic_string());
        if (auto ok = readFile(config, chosen); !ok) return unexpected(ok.error());
    }
    for (const auto& [option, value] : options) {
        if (option == "--config") continue;
        if (auto ok = applyOption(config, option, value); !ok) return unexpected(ok.error());
    }
    return config;
}

Result<EngineConfig> EngineConfig::load(const std::filesystem::path& file, int argc, char** argv) {
    std::vector<std::string_view> args;
    for (int i = 1; i < argc; ++i)
        args.emplace_back(argv[i]);
    return EngineConfig::load(file, args);
}

}
