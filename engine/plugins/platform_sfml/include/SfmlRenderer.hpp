/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SfmlRenderer
*/

#pragma once

#include "engine/render/IRenderer.hpp"
#include "engine/render/RenderQueue.hpp"
#include "engine/render/WindowDesc.hpp"

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace engine {

class SfmlRenderer final : public IRenderer {
private:
    struct FontSlot {
        std::vector<std::byte> bytes;
        std::unique_ptr<sf::Font> font;
    };

    sf::RenderTarget& _target;
    std::function<void()> _present;
    WindowDesc _desc;
    std::vector<std::unique_ptr<sf::Texture>> _textures;
    std::vector<FontSlot> _fonts;
    sf::VertexArray _batch{sf::PrimitiveType::Triangles};
    const sf::Texture* _batchTexture = nullptr;
    std::vector<uint32_t> _order;
    ColorFilter _filter = ColorFilter::None;
    RenderStats _stats;

    void applyView(const Camera2D& camera, bool screenSpace);
    void flushBatch();
    void appendSprite(const DrawCmd& cmd, const sf::Texture& texture);
    void appendQuad(const sf::Vector2f (&corners)[4], const sf::Color& color, const sf::Vector2f* uv = nullptr);
    void appendRect(const DrawCmd& cmd, bool outline);
    void appendLine(sf::Vector2f a, sf::Vector2f b, float thickness, const sf::Color& color);
    void appendCircle(const DrawCmd& cmd);
    void drawText(const DrawCmd& cmd, std::string_view arena);
    const sf::Texture* texture(TextureHandle handle) const;

public:
    explicit SfmlRenderer(sf::RenderTarget& target, std::function<void()> present = {});

    void setWindowDesc(const WindowDesc& desc);
    const WindowDesc& windowDesc() const;

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

}
