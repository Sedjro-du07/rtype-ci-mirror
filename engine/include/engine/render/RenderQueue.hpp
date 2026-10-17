/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RenderQueue
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/math/Color.hpp"
#include "engine/core/math/Rect.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/render/Camera2D.hpp"
#include "engine/render/DrawCmd.hpp"
#include "engine/render/Handles.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace engine {

struct RenderFrame {
    Camera2D camera;
    Color clearColor;
    std::span<const DrawCmd> commands;
    std::string_view textArena;
};

class ENGINE_API RenderQueue {
private:
    std::vector<DrawCmd> _commands;
    std::string _text;
    Camera2D _camera;
    Color _clearColor = Color::black();

public:
    void sprite(TextureHandle t, Rect source, Vec2 position, int16_t layer, Vec2 scale = {1, 1}, float rotation = 0,
                Color tint = {}, Vec2 origin = {});
    void text(std::string_view s, FontHandle f, uint16_t size, Vec2 position, int16_t layer, Color color = {},
              bool screenSpace = true);
    void rect(Rect r, Color c, int16_t layer, bool filled = true, bool screenSpace = false);
    void line(Vec2 a, Vec2 b, Color c, int16_t layer);
    void circle(Vec2 center, float radius, Color c, int16_t layer);
    void push(const DrawCmd& cmd);
    void setCamera(const Camera2D& c);
    const Camera2D& camera() const;
    void setClearColor(Color c);
    RenderFrame frame() const;
    void clear();
    size_t size() const;
};

}
