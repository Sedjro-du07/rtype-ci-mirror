/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RenderQueue
*/

#include "engine/render/RenderQueue.hpp"

namespace engine {

void RenderQueue::sprite(TextureHandle t, Rect source, Vec2 position, int16_t layer, Vec2 scale, float rotation,
                         Color tint, Vec2 origin) {
    DrawCmd cmd;
    cmd.kind = DrawCmd::Kind::Sprite;
    cmd.texture = t;
    cmd.source = source;
    cmd.position = position;
    cmd.layer = layer;
    cmd.scale = scale;
    cmd.rotation = rotation;
    cmd.tint = tint;
    cmd.origin = origin;
    this->_commands.push_back(cmd);
}

void RenderQueue::text(std::string_view s, FontHandle f, uint16_t size, Vec2 position, int16_t layer, Color color,
                       bool screenSpace) {
    DrawCmd cmd;
    cmd.kind = DrawCmd::Kind::Text;
    cmd.font = f;
    cmd.textSize = size;
    cmd.position = position;
    cmd.layer = layer;
    cmd.tint = color;
    cmd.screenSpace = screenSpace;
    cmd.textOffset = static_cast<uint32_t>(this->_text.size());
    cmd.textLength = static_cast<uint32_t>(s.size());
    this->_text.append(s);
    this->_commands.push_back(cmd);
}

void RenderQueue::rect(Rect r, Color c, int16_t layer, bool filled, bool screenSpace) {
    DrawCmd cmd;
    cmd.kind = filled ? DrawCmd::Kind::Rect : DrawCmd::Kind::RectOutline;
    cmd.position = Vec2{r.x, r.y};
    cmd.size = Vec2{r.w, r.h};
    cmd.tint = c;
    cmd.layer = layer;
    cmd.screenSpace = screenSpace;
    this->_commands.push_back(cmd);
}

void RenderQueue::line(Vec2 a, Vec2 b, Color c, int16_t layer) {
    DrawCmd cmd;
    cmd.kind = DrawCmd::Kind::Line;
    cmd.position = a;
    cmd.end = b;
    cmd.tint = c;
    cmd.layer = layer;
    this->_commands.push_back(cmd);
}

void RenderQueue::circle(Vec2 center, float radius, Color c, int16_t layer) {
    DrawCmd cmd;
    cmd.kind = DrawCmd::Kind::Circle;
    cmd.position = center;
    cmd.size = Vec2{radius, radius};
    cmd.tint = c;
    cmd.layer = layer;
    this->_commands.push_back(cmd);
}

void RenderQueue::push(const DrawCmd& cmd) {
    this->_commands.push_back(cmd);
}

void RenderQueue::setCamera(const Camera2D& c) {
    this->_camera = c;
}

const Camera2D& RenderQueue::camera() const {
    return this->_camera;
}

void RenderQueue::setClearColor(Color c) {
    this->_clearColor = c;
}

RenderFrame RenderQueue::frame() const {
    return RenderFrame{this->_camera, this->_clearColor, this->_commands, this->_text};
}

void RenderQueue::clear() {
    this->_commands.clear();
    this->_text.clear();
}

size_t RenderQueue::size() const {
    return this->_commands.size();
}

}
