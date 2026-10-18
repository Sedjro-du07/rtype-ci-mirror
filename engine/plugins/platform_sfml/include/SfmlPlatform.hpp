/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SfmlPlatform
*/

#pragma once

#include "SfmlKeys.hpp"
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
    sf::RenderWindow _window;
    SfmlRenderer _renderer;
    WindowDesc _desc;
    DPadState _dpad;
    Logger* _log = nullptr;

    void create(const WindowDesc& desc);
    bool windowEvent(const sf::Event& event, EventBus& events);

public:
    SfmlPlatform();

    PluginKind kind() const override;
    std::string_view name() const override;
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;
    void shutdown() noexcept override;

    Result<void> openWindow(const WindowDesc& desc) override;
    void closeWindow() override;
    bool windowOpen() const override;
    void pollEvents(InputMap& input, EventBus& events) override;
    IRenderer& renderer() override;
    void setWindow(const WindowDesc& desc) override;
    void beginDebugUi() override;
    void endDebugUi() override;
    void setClipboard(std::string_view text) override;
    std::string clipboard() const override;
};

}
