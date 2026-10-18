/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SfmlPlatform
*/

#include "SfmlPlatform.hpp"

#include "SfmlKeys.hpp"
#include "engine/events/EngineEvents.hpp"
#include "engine/events/EventBus.hpp"
#include "engine/input/InputMap.hpp"
#include "engine/module/Export.hpp"
#include "engine/services/Logger.hpp"

#include <optional>

namespace engine {

SfmlPlatform::SfmlPlatform() : _renderer(_window, [this] { this->_window.display(); }) {}

PluginKind SfmlPlatform::kind() const {
    return PluginKind::Platform;
}

std::string_view SfmlPlatform::name() const {
    return "sfml";
}

uint32_t SfmlPlatform::apiVersion() const {
    return ENGINE_PLUGIN_API_VERSION;
}

Result<void> SfmlPlatform::init(const PluginContext& ctx) {
    this->_log = &ctx.log;
    return {};
}

void SfmlPlatform::shutdown() noexcept {
    this->closeWindow();
    this->_log = nullptr;
}

void SfmlPlatform::create(const WindowDesc& desc) {
    const sf::State state = desc.fullscreen ? sf::State::Fullscreen : sf::State::Windowed;
    sf::VideoMode mode({desc.width, desc.height});
    if (desc.fullscreen) mode = sf::VideoMode::getDesktopMode();
    this->_window.create(mode, sf::String::fromUtf8(desc.title.begin(), desc.title.end()), sf::Style::Default, state);
    this->_window.setVerticalSyncEnabled(desc.vsync);
    this->_window.setKeyRepeatEnabled(false);
    this->_desc = desc;
    const sf::Vector2u size = this->_window.getSize();
    this->_desc.width = size.x;
    this->_desc.height = size.y;
    this->_renderer.setWindowDesc(this->_desc);
}

Result<void> SfmlPlatform::openWindow(const WindowDesc& desc) {
    if (desc.width == 0 || desc.height == 0 || desc.virtualWidth == 0 || desc.virtualHeight == 0)
        return fail(Errc::ConfigInvalid, "window and virtual sizes must be greater than 0");
    this->create(desc);
    if (!this->_window.isOpen()) return fail(Errc::ConfigInvalid, "SFML could not open the window");
    if (this->_log != nullptr)
        this->_log->info("window {}x{} (virtual {}x{}){}", this->_desc.width, this->_desc.height,
                         this->_desc.virtualWidth, this->_desc.virtualHeight,
                         this->_desc.fullscreen ? ", fullscreen" : "");
    return {};
}

void SfmlPlatform::closeWindow() {
    if (this->_window.isOpen()) this->_window.close();
}

bool SfmlPlatform::windowOpen() const {
    return this->_window.isOpen();
}

bool SfmlPlatform::windowEvent(const sf::Event& event, EventBus& events) {
    if (event.is<sf::Event::Closed>()) {
        events.publish(QuitRequested{});
        return true;
    }
    if (const auto* resized = event.getIf<sf::Event::Resized>()) {
        this->_desc.width = resized->size.x;
        this->_desc.height = resized->size.y;
        this->_renderer.setWindowDesc(this->_desc);
        events.publish(WindowResized{resized->size.x, resized->size.y});
        return true;
    }
    if (event.is<sf::Event::FocusLost>() || event.is<sf::Event::FocusGained>()) {
        events.publish(WindowFocusChanged{event.is<sf::Event::FocusGained>()});
        return true;
    }
    return false;
}

void SfmlPlatform::pollEvents(InputMap& input, EventBus& events) {
    if (!this->_window.isOpen()) return;
    while (const std::optional<sf::Event> event = this->_window.pollEvent()) {
        if (this->windowEvent(*event, events)) continue;
        for (const RawInputEvent& raw : toRawInputEvents(*event, this->_desc, this->_dpad))
            input.feed(raw);
    }
}

IRenderer& SfmlPlatform::renderer() {
    return this->_renderer;
}

void SfmlPlatform::setWindow(const WindowDesc& desc) {
    if (!this->_window.isOpen()) {
        this->_desc = desc;
        this->_renderer.setWindowDesc(desc);
        return;
    }
    const bool recreate = desc.fullscreen != this->_desc.fullscreen;
    if (recreate) {
        this->create(desc);
        return;
    }
    if (desc.title != this->_desc.title)
        this->_window.setTitle(sf::String::fromUtf8(desc.title.begin(), desc.title.end()));
    if (desc.vsync != this->_desc.vsync) this->_window.setVerticalSyncEnabled(desc.vsync);
    if (desc.width != this->_desc.width || desc.height != this->_desc.height)
        this->_window.setSize({desc.width, desc.height});
    this->_desc = desc;
    const sf::Vector2u size = this->_window.getSize();
    this->_desc.width = size.x;
    this->_desc.height = size.y;
    this->_renderer.setWindowDesc(this->_desc);
}

void SfmlPlatform::beginDebugUi() {}

void SfmlPlatform::endDebugUi() {}

void SfmlPlatform::setClipboard(std::string_view text) {
    sf::Clipboard::setString(sf::String::fromUtf8(text.begin(), text.end()));
}

std::string SfmlPlatform::clipboard() const {
    const auto utf8 = sf::Clipboard::getString().toUtf8();
    return std::string(utf8.begin(), utf8.end());
}

}

ENGINE_PLUGIN(engine::SfmlPlatform)
