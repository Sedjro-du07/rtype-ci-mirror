/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** NullPlugins
*/

#include "engine/module/NullPlugins.hpp"

#include "engine/module/Export.hpp"

#include <cstddef>
#include <utility>

namespace engine {

Result<TextureHandle> NullRenderer::createTexture(std::span<const std::byte>, std::string_view) {
    return TextureHandle{this->_next++};
}

void NullRenderer::destroyTexture(TextureHandle) {}

Vec2 NullRenderer::textureSize(TextureHandle) const {
    return {};
}

Result<FontHandle> NullRenderer::createFont(std::span<const std::byte>, std::string_view) {
    return FontHandle{this->_next++};
}

void NullRenderer::destroyFont(FontHandle) {}

void NullRenderer::submit(const RenderFrame&) {}

bool NullRenderer::supports(Feature) const {
    return false;
}

void NullRenderer::setColorFilter(ColorFilter) {}

RenderStats NullRenderer::stats() const {
    return {};
}

PluginKind NullPlatform::kind() const {
    return PluginKind::Platform;
}

std::string_view NullPlatform::name() const {
    return "null";
}

uint32_t NullPlatform::apiVersion() const {
    return ENGINE_PLUGIN_API_VERSION;
}

Result<void> NullPlatform::init(const PluginContext&) {
    return {};
}

void NullPlatform::shutdown() noexcept {
    this->_open = false;
}

Result<void> NullPlatform::openWindow(const WindowDesc&) {
    this->_open = true;
    return {};
}

void NullPlatform::closeWindow() {
    this->_open = false;
}

bool NullPlatform::windowOpen() const {
    return this->_open;
}

void NullPlatform::pollEvents(InputMap&, EventBus&) {}

IRenderer& NullPlatform::renderer() {
    return this->_renderer;
}

void NullPlatform::setWindow(const WindowDesc&) {}

void NullPlatform::beginDebugUi() {}

void NullPlatform::endDebugUi() {}

void NullPlatform::setClipboard(std::string_view text) {
    this->_clipboard = std::string(text);
}

std::string NullPlatform::clipboard() const {
    return this->_clipboard;
}

PluginKind NullAudio::kind() const {
    return PluginKind::Audio;
}

std::string_view NullAudio::name() const {
    return "null";
}

uint32_t NullAudio::apiVersion() const {
    return ENGINE_PLUGIN_API_VERSION;
}

Result<void> NullAudio::init(const PluginContext&) {
    return {};
}

void NullAudio::shutdown() noexcept {}

Result<SoundHandle> NullAudio::createSound(std::span<const std::byte>, std::string_view) {
    return SoundHandle{this->_next++};
}

void NullAudio::destroySound(SoundHandle) {}

Result<MusicHandle> NullAudio::openMusic(const std::filesystem::path&) {
    return MusicHandle{this->_next++};
}

void NullAudio::closeMusic(MusicHandle) {}

VoiceId NullAudio::play(SoundHandle, const PlayParams&) {
    return 0;
}

void NullAudio::stop(VoiceId) {}

void NullAudio::setVoiceVolume(VoiceId, float) {}

void NullAudio::playMusic(MusicHandle, float) {}

void NullAudio::stopMusic(float) {}

void NullAudio::setBusVolume(AudioBus bus, float volume) {
    const auto index = static_cast<size_t>(bus);
    if (index < this->_buses.size()) this->_buses[index] = volume < 0.f ? 0.f : (volume > 1.f ? 1.f : volume);
}

float NullAudio::busVolume(AudioBus bus) const {
    const auto index = static_cast<size_t>(bus);
    return index < this->_buses.size() ? this->_buses[index] : 1.f;
}

void NullAudio::setListener(Vec2, float) {}

void NullAudio::update() {}

PluginKind NullNetwork::kind() const {
    return PluginKind::Network;
}

std::string_view NullNetwork::name() const {
    return "null";
}

uint32_t NullNetwork::apiVersion() const {
    return ENGINE_PLUGIN_API_VERSION;
}

Result<void> NullNetwork::init(const PluginContext&) {
    return {};
}

void NullNetwork::shutdown() noexcept {}

Result<void> NullNetwork::openUdp(uint16_t) {
    return {};
}

void NullNetwork::closeUdp() {}

void NullNetwork::sendUdp(const Endpoint&, std::span<const std::byte>) {}

void NullNetwork::setReceiver(std::move_only_function<void(Datagram&&)>) {}

Result<void> NullNetwork::listenTcp(uint16_t) {
    return {};
}

Result<uint32_t> NullNetwork::connectTcp(const Endpoint& to) {
    return fail(Errc::NetworkError, "the null network plugin cannot connect to " + to.toString());
}

void NullNetwork::sendTcp(uint32_t, std::span<const std::byte>) {}

void NullNetwork::setTcpReceiver(std::move_only_function<void(uint32_t, std::span<const std::byte>)>) {}

void NullNetwork::closeTcp(uint32_t) {}

PluginKind NullScripting::kind() const {
    return PluginKind::Scripting;
}

std::string_view NullScripting::name() const {
    return "null";
}

uint32_t NullScripting::apiVersion() const {
    return ENGINE_PLUGIN_API_VERSION;
}

Result<void> NullScripting::init(const PluginContext&) {
    return {};
}

void NullScripting::shutdown() noexcept {}

Result<void> NullScripting::bind(ScriptHost&) {
    return {};
}

Result<ScriptHandle> NullScripting::load(const std::filesystem::path& file) {
    return fail(Errc::ScriptError, "scripting is disabled (scripting: null): cannot run " + file.generic_string());
}

void NullScripting::unload(ScriptHandle) {}

Result<void> NullScripting::reload(ScriptHandle) {
    return fail(Errc::ScriptError, "scripting is disabled (scripting: null)");
}

Result<void> NullScripting::call(ScriptHandle, std::string_view, Entity, Context&) {
    return fail(Errc::ScriptError, "scripting is disabled (scripting: null)");
}

bool NullScripting::has(ScriptHandle, std::string_view) const {
    return false;
}

std::unique_ptr<IPlugin> makeNullPlugin(PluginKind kind) {
    switch (kind) {
    case PluginKind::Audio:
        return std::make_unique<NullAudio>();
    case PluginKind::Network:
        return std::make_unique<NullNetwork>();
    case PluginKind::Scripting:
        return std::make_unique<NullScripting>();
    case PluginKind::Platform:
        break;
    }
    return std::make_unique<NullPlatform>();
}

}
