/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** IRenderer
*/

#pragma once

#include "engine/core/Error.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/render/Handles.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace engine {

struct RenderFrame;

enum class Feature : uint8_t { Render3D, ColorFilter, Shaders };
enum class ColorFilter : uint8_t { None, Protanopia, Deuteranopia, Tritanopia, HighContrast };

struct RenderStats {
    uint32_t commands = 0, batches = 0, drawCalls = 0;
    double submitMs = 0;
};

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual Result<TextureHandle> createTexture(std::span<const std::byte> encodedImage,
                                                std::string_view debugName) = 0;
    virtual void destroyTexture(TextureHandle texture) = 0;
    virtual Vec2 textureSize(TextureHandle texture) const = 0;
    virtual Result<FontHandle> createFont(std::span<const std::byte> fontFile, std::string_view debugName) = 0;
    virtual void destroyFont(FontHandle font) = 0;
    virtual void submit(const RenderFrame& frame) = 0;
    virtual bool supports(Feature feature) const = 0;
    virtual void setColorFilter(ColorFilter filter) = 0;
    virtual RenderStats stats() const = 0;
};

}
