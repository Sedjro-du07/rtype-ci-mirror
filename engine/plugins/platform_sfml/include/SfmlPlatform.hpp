/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SfmlPlatform
*/

// engine/plugins/platform_sfml/include/SfmlPlatform.hpp
// ENG-31 — platform_sfml plugin: window, events to InputMap, batching renderer
// Spec: docs/engine/reference/08-rendering.md · ADR 0007, 0031

#pragma once

#include "SfmlRenderer.hpp"
#include "engine/render/IPlatform.hpp"
#include "engine/render/WindowDesc.hpp"

#include <SFML/Graphics.hpp>

#include <string>
#include <string_view>

namespace engine {

class Logger;

class SfmlPlatform final : public IPlatform {
private:
    sf::RenderWindow _window; // declared before the renderer, which keeps a reference to it
    SfmlRenderer _renderer;
    WindowDesc _desc;
    Logger* _log = nullptr;

    void create(const WindowDesc& desc);

public:
    SfmlPlatform();

    // IPlugin
    PluginKind kind() const override;       // Platform
    std::string_view name() const override; // "sfml"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;
    void shutdown() noexcept override;

    // IPlatform
    Result<void> openWindow(const WindowDesc& desc) override;
    void closeWindow() override;
    bool windowOpen() const override;
    void pollEvents(InputMap& input, EventBus& events) override; // input → InputMap::feed, window → EventBus
    IRenderer& renderer() override;
    void setWindow(const WindowDesc& desc) override;
    void beginDebugUi() override; // no-op (ADR 0046)
    void endDebugUi() override;   // no-op (ADR 0046)
    void setClipboard(std::string_view text) override;
    std::string clipboard() const override;
};

}
