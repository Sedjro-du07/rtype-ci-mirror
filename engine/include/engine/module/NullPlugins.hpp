/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** NullPlugins
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/audio/IAudio.hpp"
#include "engine/net/INetwork.hpp"
#include "engine/render/IPlatform.hpp"
#include "engine/render/IRenderer.hpp"
#include "engine/script/IScripting.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace engine {

class ENGINE_API NullRenderer final : public IRenderer {
private:
    uint32_t _next = 1;

public:
    Result<TextureHandle> createTexture(std::span<const std::byte> encodedImage, std::string_view debugName) override;
    void destroyTexture(TextureHandle texture) override;
    Vec2 textureSize(TextureHandle texture) const override;
    Result<FontHandle> createFont(std::span<const std::byte> fontFile, std::string_view debugName) override;
    void destroyFont(FontHandle font) override;
    void submit(const RenderFrame& frame) override;
    bool supports(Feature feature) const override;
    void setColorFilter(ColorFilter filter) override;
    RenderStats stats() const override;
};

class ENGINE_API NullPlatform final : public IPlatform {
private:
    NullRenderer _renderer;
    bool _open = false;
    std::string _clipboard;

public:
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

class ENGINE_API NullAudio final : public IAudio {
private:
    uint32_t _next = 1;
    std::array<float, 4> _buses{1.f, 1.f, 1.f, 1.f};

public:
    PluginKind kind() const override;
    std::string_view name() const override;
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;
    void shutdown() noexcept override;
    Result<SoundHandle> createSound(std::span<const std::byte> encoded, std::string_view debugName) override;
    void destroySound(SoundHandle sound) override;
    Result<MusicHandle> openMusic(const std::filesystem::path& file) override;
    void closeMusic(MusicHandle music) override;
    VoiceId play(SoundHandle sound, const PlayParams& params) override;
    void stop(VoiceId voice) override;
    void setVoiceVolume(VoiceId voice, float volume) override;
    void playMusic(MusicHandle music, float fadeInSeconds) override;
    void stopMusic(float fadeOutSeconds) override;
    void setBusVolume(AudioBus bus, float volume) override;
    float busVolume(AudioBus bus) const override;
    void setListener(Vec2 position, float hearingRadius) override;
    void update() override;
};

class ENGINE_API NullNetwork final : public INetwork {
public:
    PluginKind kind() const override;
    std::string_view name() const override;
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;
    void shutdown() noexcept override;
    Result<void> openUdp(uint16_t port) override;
    void closeUdp() override;
    void sendUdp(const Endpoint& to, std::span<const std::byte> bytes) override;
    void setReceiver(std::move_only_function<void(Datagram&&)> onDatagram) override;
    Result<void> listenTcp(uint16_t port) override;
    Result<uint32_t> connectTcp(const Endpoint& to) override;
    void sendTcp(uint32_t stream, std::span<const std::byte> bytes) override;
    void setTcpReceiver(std::move_only_function<void(uint32_t, std::span<const std::byte>)> fn) override;
    void closeTcp(uint32_t stream) override;
};

class ENGINE_API NullScripting final : public IScripting {
public:
    PluginKind kind() const override;
    std::string_view name() const override;
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;
    void shutdown() noexcept override;
    Result<void> bind(ScriptHost& host) override;
    Result<ScriptHandle> load(const std::filesystem::path& file) override;
    void unload(ScriptHandle script) override;
    Result<void> reload(ScriptHandle script) override;
    Result<void> call(ScriptHandle script, std::string_view function, Entity self, Context& ctx) override;
    bool has(ScriptHandle script, std::string_view function) const override;
};

ENGINE_API std::unique_ptr<IPlugin> makeNullPlugin(PluginKind kind);

}
