/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** FixturePlugin
*/

#include "engine/module/Export.hpp"

#include <cstdint>
#include <string_view>

namespace fixture {

class FixturePlugin final : public engine::IPlugin {
public:
    engine::PluginKind kind() const override { return engine::PluginKind::Network; }
    std::string_view name() const override { return "fixture"; }
    uint32_t apiVersion() const override { return engine::ENGINE_PLUGIN_API_VERSION; }
    engine::Result<void> init(const engine::PluginContext&) override { return {}; }
    void shutdown() noexcept override {}
};

}

ENGINE_PLUGIN(fixture::FixturePlugin)
