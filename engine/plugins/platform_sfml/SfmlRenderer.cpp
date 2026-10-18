/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SfmlRenderer
*/

#include "SfmlRenderer.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <numbers>
#include <string>
#include <tuple>
#include <utility>

namespace engine {

namespace {

constexpr int CircleSegments = 32;

sf::Color toSfml(Color c) {
    return sf::Color(c.r, c.g, c.b, c.a);
}

sf::Vector2f toSfml(Vec2 v) {
    return {v.x, v.y};
}

sf::Vector2f rotate(sf::Vector2f v, float degrees) {
    const float radians = degrees * std::numbers::pi_v<float> / 180.f;
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    return {v.x * c - v.y * s, v.x * s + v.y * c};
}

template <class Slot>
uint32_t firstFree(std::vector<Slot>& slots, auto&& isFree) {
    for (size_t i = 0; i < slots.size(); ++i)
        if (isFree(slots[i])) return static_cast<uint32_t>(i);
    slots.emplace_back();
    return static_cast<uint32_t>(slots.size() - 1);
}

auto sortKey(const DrawCmd& cmd) {
    const bool text = cmd.kind == DrawCmd::Kind::Text;
    const uint32_t texture = cmd.kind == DrawCmd::Kind::Sprite ? cmd.texture.id : 0;
    return std::tuple{cmd.layer, cmd.screenSpace, text, texture};
}

}

SfmlRenderer::SfmlRenderer(sf::RenderTarget& target, std::function<void()> present)
    : _target(target), _present(std::move(present)) {}

void SfmlRenderer::setWindowDesc(const WindowDesc& desc) {
    this->_desc = desc;
}

const WindowDesc& SfmlRenderer::windowDesc() const {
    return this->_desc;
}

Result<TextureHandle> SfmlRenderer::createTexture(std::span<const std::byte> encodedImage, std::string_view debugName) {
    auto texture = std::make_unique<sf::Texture>();
    if (encodedImage.empty() || !texture->loadFromMemory(encodedImage.data(), encodedImage.size()))
        return fail(Errc::AssetLoadFailed, "cannot decode texture '" + std::string(debugName) + "'");
    const uint32_t index = firstFree(this->_textures, [](const auto& slot) { return slot == nullptr; });
    this->_textures[index] = std::move(texture);
    return TextureHandle{index + 1};
}

void SfmlRenderer::destroyTexture(TextureHandle texture) {
    if (texture.valid() && texture.id <= this->_textures.size()) this->_textures[texture.id - 1].reset();
}

const sf::Texture* SfmlRenderer::texture(TextureHandle handle) const {
    if (!handle.valid() || handle.id > this->_textures.size()) return nullptr;
    return this->_textures[handle.id - 1].get();
}

Vec2 SfmlRenderer::textureSize(TextureHandle texture) const {
    const sf::Texture* found = this->texture(texture);
    if (found == nullptr) return {};
    const sf::Vector2u size = found->getSize();
    return Vec2{static_cast<float>(size.x), static_cast<float>(size.y)};
}

Result<FontHandle> SfmlRenderer::createFont(std::span<const std::byte> fontFile, std::string_view debugName) {
    FontSlot slot;
    slot.bytes.assign(fontFile.begin(), fontFile.end());
    slot.font = std::make_unique<sf::Font>();
    if (slot.bytes.empty() || !slot.font->openFromMemory(slot.bytes.data(), slot.bytes.size()))
        return fail(Errc::AssetLoadFailed, "cannot open font '" + std::string(debugName) + "'");
    const uint32_t index = firstFree(this->_fonts, [](const FontSlot& s) { return s.font == nullptr; });
    this->_fonts[index] = std::move(slot);
    return FontHandle{index + 1};
}

void SfmlRenderer::destroyFont(FontHandle font) {
    if (font.valid() && font.id <= this->_fonts.size()) this->_fonts[font.id - 1] = FontSlot{};
}

bool SfmlRenderer::supports(Feature feature) const {
    (void)feature;
    return false;
}

void SfmlRenderer::setColorFilter(ColorFilter filter) {
    this->_filter = filter;
}

RenderStats SfmlRenderer::stats() const {
    return this->_stats;
}

void SfmlRenderer::applyView(const Camera2D& camera, bool screenSpace) {
    const sf::Vector2u size = this->_target.getSize();
    const float vw = static_cast<float>(std::max(1u, this->_desc.virtualWidth));
    const float vh = static_cast<float>(std::max(1u, this->_desc.virtualHeight));
    const float w = static_cast<float>(std::max(1u, size.x));
    const float h = static_cast<float>(std::max(1u, size.y));
    const float scale = std::min(w / vw, h / vh);
    const sf::FloatRect viewport({(w - vw * scale) / 2.f / w, (h - vh * scale) / 2.f / h},
                                 {vw * scale / w, vh * scale / h});
    sf::View view;
    if (screenSpace) {
        view = sf::View(sf::FloatRect({0.f, 0.f}, {vw, vh}));
    } else {
        const float zoom = camera.zoom > 0.f ? camera.zoom : 1.f;
        view = sf::View(toSfml(camera.center), {vw / zoom, vh / zoom});
        view.setRotation(sf::degrees(camera.rotation));
    }
    view.setViewport(viewport);
    this->_target.setView(view);
}

void SfmlRenderer::flushBatch() {
    if (this->_batch.getVertexCount() == 0) return;
    sf::RenderStates states;
    states.texture = this->_batchTexture;
    this->_target.draw(this->_batch, states);
    this->_batch.clear();
    ++this->_stats.batches;
    ++this->_stats.drawCalls;
}

void SfmlRenderer::appendQuad(const sf::Vector2f (&corners)[4], const sf::Color& color, const sf::Vector2f* uv) {
    static constexpr int Triangles[6] = {0, 1, 2, 0, 2, 3};
    for (const int i : Triangles)
        this->_batch.append(sf::Vertex{corners[i], color, uv != nullptr ? uv[i] : sf::Vector2f{}});
}

void SfmlRenderer::appendSprite(const DrawCmd& cmd, const sf::Texture& texture) {
    Rect source = cmd.source;
    if (source.w == 0.f || source.h == 0.f) {
        const sf::Vector2u size = texture.getSize();
        source = Rect{0.f, 0.f, static_cast<float>(size.x), static_cast<float>(size.y)};
    }
    const sf::Vector2f local[4] = {{0.f, 0.f}, {source.w, 0.f}, {source.w, source.h}, {0.f, source.h}};
    sf::Vector2f corners[4];
    sf::Vector2f uv[4];
    for (int i = 0; i < 4; ++i) {
        const sf::Vector2f pivoted{(local[i].x - cmd.origin.x) * cmd.scale.x,
                                   (local[i].y - cmd.origin.y) * cmd.scale.y};
        corners[i] = toSfml(cmd.position) + rotate(pivoted, cmd.rotation);
        uv[i] = {source.x + local[i].x, source.y + local[i].y};
    }
    this->appendQuad(corners, toSfml(cmd.tint), uv);
}

void SfmlRenderer::appendLine(sf::Vector2f a, sf::Vector2f b, float thickness, const sf::Color& color) {
    const sf::Vector2f d = b - a;
    const float length = std::sqrt(d.x * d.x + d.y * d.y);
    if (length == 0.f) return;
    const sf::Vector2f n{-d.y / length * thickness / 2.f, d.x / length * thickness / 2.f};
    const sf::Vector2f corners[4] = {a + n, b + n, b - n, a - n};
    this->appendQuad(corners, color);
}

void SfmlRenderer::appendRect(const DrawCmd& cmd, bool outline) {
    const sf::Vector2f p = toSfml(cmd.position);
    const sf::Vector2f s = toSfml(cmd.size);
    const sf::Color color = toSfml(cmd.tint);
    if (!outline) {
        const sf::Vector2f corners[4] = {p, {p.x + s.x, p.y}, p + s, {p.x, p.y + s.y}};
        this->appendQuad(corners, color);
        return;
    }
    const sf::Vector2f tl = p, tr{p.x + s.x, p.y}, br = p + s, bl{p.x, p.y + s.y};
    const sf::Vector2f edges[4][4] = {{tl, tr, {tr.x, tr.y + 1}, {tl.x, tl.y + 1}},
                                      {{br.x, br.y - 1}, {bl.x, bl.y - 1}, bl, br},
                                      {tl, {tl.x + 1, tl.y}, {bl.x + 1, bl.y}, bl},
                                      {{tr.x - 1, tr.y}, tr, br, {br.x - 1, br.y}}};
    for (const auto& edge : edges)
        this->appendQuad(edge, color);
}

void SfmlRenderer::appendCircle(const DrawCmd& cmd) {
    const sf::Vector2f center = toSfml(cmd.position);
    const float radius = cmd.size.x;
    const sf::Color color = toSfml(cmd.tint);
    for (int i = 0; i < CircleSegments; ++i) {
        const float a0 = 2.f * std::numbers::pi_v<float> * static_cast<float>(i) / CircleSegments;
        const float a1 = 2.f * std::numbers::pi_v<float> * static_cast<float>(i + 1) / CircleSegments;
        this->_batch.append(sf::Vertex{center, color});
        this->_batch.append(sf::Vertex{center + sf::Vector2f{std::cos(a0), std::sin(a0)} * radius, color});
        this->_batch.append(sf::Vertex{center + sf::Vector2f{std::cos(a1), std::sin(a1)} * radius, color});
    }
}

void SfmlRenderer::drawText(const DrawCmd& cmd, std::string_view arena) {
    if (!cmd.font.valid() || cmd.font.id > this->_fonts.size()) return;
    const sf::Font* font = this->_fonts[cmd.font.id - 1].font.get();
    if (font == nullptr || static_cast<size_t>(cmd.textOffset) + cmd.textLength > arena.size()) return;
    const std::string_view utf8 = arena.substr(cmd.textOffset, cmd.textLength);
    sf::Text text(*font, sf::String::fromUtf8(utf8.begin(), utf8.end()), cmd.textSize);
    text.setFillColor(toSfml(cmd.tint));
    text.setPosition(toSfml(cmd.position));
    text.setScale(toSfml(cmd.scale));
    text.setRotation(sf::degrees(cmd.rotation));
    this->_target.draw(text);
    ++this->_stats.batches;
    ++this->_stats.drawCalls;
}

void SfmlRenderer::submit(const RenderFrame& frame) {
    const auto start = std::chrono::steady_clock::now();
    this->_stats = RenderStats{};
    this->_stats.commands = static_cast<uint32_t>(frame.commands.size());

    this->_target.clear(sf::Color::Black);
    this->applyView(frame.camera, true);
    const float vw = static_cast<float>(this->_desc.virtualWidth);
    const float vh = static_cast<float>(this->_desc.virtualHeight);
    const sf::Vector2f screen[4] = {{0.f, 0.f}, {vw, 0.f}, {vw, vh}, {0.f, vh}};
    this->_batchTexture = nullptr;
    this->appendQuad(screen, toSfml(frame.clearColor));
    this->flushBatch();
    this->_stats.batches = 0;
    this->_stats.drawCalls = 0;

    this->_order.resize(frame.commands.size());
    for (uint32_t i = 0; i < this->_order.size(); ++i)
        this->_order[i] = i;
    std::stable_sort(this->_order.begin(), this->_order.end(),
                     [&](uint32_t a, uint32_t b) { return sortKey(frame.commands[a]) < sortKey(frame.commands[b]); });

    bool haveView = false;
    bool viewScreenSpace = false;
    for (const uint32_t index : this->_order) {
        const DrawCmd& cmd = frame.commands[index];
        const sf::Texture* wanted = nullptr;
        if (cmd.kind == DrawCmd::Kind::Sprite) {
            wanted = this->texture(cmd.texture);
            if (wanted == nullptr) continue;
        }
        if (!haveView || viewScreenSpace != cmd.screenSpace) {
            this->flushBatch();
            this->applyView(frame.camera, cmd.screenSpace);
            haveView = true;
            viewScreenSpace = cmd.screenSpace;
        }
        if (cmd.kind == DrawCmd::Kind::Text) {
            this->flushBatch();
            this->drawText(cmd, frame.textArena);
            continue;
        }
        if (wanted != this->_batchTexture) {
            this->flushBatch();
            this->_batchTexture = wanted;
        }
        switch (cmd.kind) {
        case DrawCmd::Kind::Sprite:
            this->appendSprite(cmd, *wanted);
            break;
        case DrawCmd::Kind::Rect:
            this->appendRect(cmd, false);
            break;
        case DrawCmd::Kind::RectOutline:
            this->appendRect(cmd, true);
            break;
        case DrawCmd::Kind::Line:
            this->appendLine(toSfml(cmd.position), toSfml(cmd.end), 1.f, toSfml(cmd.tint));
            break;
        case DrawCmd::Kind::Circle:
            this->appendCircle(cmd);
            break;
        case DrawCmd::Kind::Text:
            break;
        }
    }
    this->flushBatch();
    this->_batchTexture = nullptr;
    if (this->_present) this->_present();
    this->_stats.submitMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

}
