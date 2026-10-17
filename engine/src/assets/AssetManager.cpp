/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** AssetManager
*/

#include "engine/assets/AssetManager.hpp"

#include "engine/audio/IAudio.hpp"
#include "engine/render/IRenderer.hpp"
#include "engine/script/IScripting.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <span>
#include <system_error>
#include <utility>
#include <vector>

namespace engine {

namespace {

constexpr std::array<unsigned char, 90> MissingTexturePng{
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00,
    0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0x0f, 0xbe, 0x8b, 0x00, 0x00, 0x00,
    0x21, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0x63, 0xf8, 0xcf, 0xf0, 0xff, 0x3f, 0x08, 0x33, 0x30, 0x30, 0x80,
    0x31, 0x3a, 0x9f, 0x81, 0xa0, 0x02, 0x5c, 0x12, 0x48, 0x7c, 0x02, 0x0a, 0x06, 0x81, 0x1b, 0x00, 0x83, 0x17,
    0x7f, 0x81, 0x9a, 0x6e, 0xa6, 0x20, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

std::span<const std::byte> asBytes(std::string_view text) {
    return std::as_bytes(std::span(text.data(), text.size()));
}

template <class Tag>
std::string_view pathIn(const auto& table, Handle<Tag> handle) {
    const auto it = table.byId.find(handle.id);
    return it == table.byId.end() ? std::string_view{} : std::string_view(it->second);
}

template <class Tag>
void addRef(auto& table, Handle<Tag> handle, int delta) {
    const auto it = table.byId.find(handle.id);
    if (it == table.byId.end()) return;
    auto& entry = table.byPath.at(it->second);
    if (delta > 0)
        ++entry.refs;
    else if (entry.refs > 0)
        --entry.refs;
}

template <class Tag>
Handle<Tag> cached(const auto& table, std::string_view path) {
    const auto it = table.byPath.find(std::string(path));
    return it == table.byPath.end() ? Handle<Tag>{} : Handle<Tag>{it->second.id};
}

void remember(auto& table, std::string_view path, uint32_t id, uint64_t bytes, std::filesystem::file_time_type time) {
    table.byPath[std::string(path)] = {id, 0, bytes, time};
    table.byId[id] = std::string(path);
}

}

AssetManager::AssetManager(Logger& log, std::filesystem::path root)
    : _log(log.tagged("assets")), _root(std::move(root)) {}

void AssetManager::attach(IRenderer& renderer, IAudio& audio, IScripting& scripting) {
    this->_renderer = &renderer;
    this->_audio = &audio;
    this->_scripting = &scripting;
}

std::filesystem::path AssetManager::resolve(std::string_view path) const {
    const std::filesystem::path relative(path);
    if (path.empty() || relative.is_absolute() || relative.has_root_name()) return {};
    for (const auto& part : relative)
        if (part == "..") return {};
    std::filesystem::path current = this->_root;
    std::error_code error;
    for (const auto& part : relative) {
        if (part == ".") continue;
        const std::filesystem::path next = current / part;
        if (std::filesystem::is_directory(current, error)) {
            bool exact = false, other = false;
            for (const auto& item : std::filesystem::directory_iterator(current, error)) {
                const std::string name = item.path().filename().string();
                if (name == part.string()) {
                    exact = true;
                    break;
                }
                if (name.size() == part.string().size() &&
                    std::equal(name.begin(), name.end(), part.string().begin(),
                               [](char a, char b) { return std::tolower(a) == std::tolower(b); }))
                    other = true;
            }
            if (!exact && other) return {};
        }
        current = next;
    }
    return current;
}

void AssetManager::fail(std::string_view kind, std::string_view path, std::string_view reason) {
    const std::string key = std::string(kind) + ':' + std::string(path);
    if (this->_failed.contains(key)) return;
    this->_failed[key] = 1;
    this->_log.error("cannot load {} '{}': {}", kind, path, reason);
}

bool AssetManager::readFile(std::string_view path, std::filesystem::path& resolved, std::string& bytes) {
    resolved = this->resolve(path);
    if (resolved.empty()) return false;
    std::ifstream in(resolved, std::ios::binary);
    if (!in) return false;
    bytes.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    return true;
}

TextureHandle AssetManager::missingTexture() {
    if (!this->_missing.valid() && this->_renderer) {
        const auto bytes = std::as_bytes(std::span(MissingTexturePng));
        if (const auto created = this->_renderer->createTexture(bytes, "missing")) this->_missing = *created;
    }
    return this->_missing;
}

TextureHandle AssetManager::texture(std::string_view path) {
    if (const TextureHandle handle = cached<TextureTag>(this->_textures, path); handle.valid()) return handle;
    if (!this->_renderer) {
        this->fail("texture", path, "no renderer attached");
        return {};
    }
    std::filesystem::path resolved;
    std::string bytes;
    if (!this->readFile(path, resolved, bytes)) {
        this->fail("texture", path, "file not found, outside the game root or with a different case");
        return this->missingTexture();
    }
    const auto created = this->_renderer->createTexture(asBytes(bytes), path);
    if (!created) {
        this->fail("texture", path, created.error().message);
        return this->missingTexture();
    }
    std::error_code error;
    remember(this->_textures, path, created->id, bytes.size(), std::filesystem::last_write_time(resolved, error));
    return *created;
}

FontHandle AssetManager::font(std::string_view path) {
    if (const FontHandle handle = cached<FontTag>(this->_fonts, path); handle.valid()) return handle;
    std::filesystem::path resolved;
    std::string bytes;
    if (!this->_renderer || !this->readFile(path, resolved, bytes)) {
        this->fail("font", path, "file not found, outside the game root or with a different case");
        return {};
    }
    const auto created = this->_renderer->createFont(asBytes(bytes), path);
    if (!created) {
        this->fail("font", path, created.error().message);
        return {};
    }
    std::error_code error;
    remember(this->_fonts, path, created->id, bytes.size(), std::filesystem::last_write_time(resolved, error));
    return *created;
}

SoundHandle AssetManager::sound(std::string_view path) {
    if (const SoundHandle handle = cached<SoundTag>(this->_sounds, path); handle.valid()) return handle;
    std::filesystem::path resolved;
    std::string bytes;
    if (!this->_audio || !this->readFile(path, resolved, bytes)) {
        this->fail("sound", path, "file not found, outside the game root or with a different case");
        return {};
    }
    const auto created = this->_audio->createSound(asBytes(bytes), path);
    if (!created) {
        this->fail("sound", path, created.error().message);
        return {};
    }
    std::error_code error;
    remember(this->_sounds, path, created->id, bytes.size(), std::filesystem::last_write_time(resolved, error));
    return *created;
}

MusicHandle AssetManager::music(std::string_view path) {
    if (const MusicHandle handle = cached<MusicTag>(this->_musics, path); handle.valid()) return handle;
    const std::filesystem::path resolved = this->resolve(path);
    std::error_code error;
    if (!this->_audio || resolved.empty() || !std::filesystem::exists(resolved, error)) {
        this->fail("music", path, "file not found, outside the game root or with a different case");
        return {};
    }
    const auto opened = this->_audio->openMusic(resolved);
    if (!opened) {
        this->fail("music", path, opened.error().message);
        return {};
    }
    remember(this->_musics, path, opened->id, 0, std::filesystem::last_write_time(resolved, error));
    return *opened;
}

ScriptHandle AssetManager::script(std::string_view path) {
    if (const ScriptHandle handle = cached<ScriptTag>(this->_scripts, path); handle.valid()) return handle;
    const std::filesystem::path resolved = this->resolve(path);
    std::error_code error;
    if (!this->_scripting || resolved.empty() || !std::filesystem::exists(resolved, error)) {
        this->fail("script", path, "file not found, outside the game root or with a different case");
        return {};
    }
    const auto loaded = this->_scripting->load(resolved);
    if (!loaded) {
        this->fail("script", path, loaded.error().toString());
        return {};
    }
    remember(this->_scripts, path, loaded->id, 0, std::filesystem::last_write_time(resolved, error));
    return *loaded;
}

std::string_view AssetManager::pathOf(TextureHandle handle) const {
    return pathIn(this->_textures, handle);
}

std::string_view AssetManager::pathOf(FontHandle handle) const {
    return pathIn(this->_fonts, handle);
}

std::string_view AssetManager::pathOf(SoundHandle handle) const {
    return pathIn(this->_sounds, handle);
}

std::string_view AssetManager::pathOf(MusicHandle handle) const {
    return pathIn(this->_musics, handle);
}

std::string_view AssetManager::pathOf(ScriptHandle handle) const {
    return pathIn(this->_scripts, handle);
}

void AssetManager::retain(TextureHandle handle) {
    addRef(this->_textures, handle, 1);
}

void AssetManager::retain(FontHandle handle) {
    addRef(this->_fonts, handle, 1);
}

void AssetManager::retain(SoundHandle handle) {
    addRef(this->_sounds, handle, 1);
}

void AssetManager::retain(MusicHandle handle) {
    addRef(this->_musics, handle, 1);
}

void AssetManager::retain(ScriptHandle handle) {
    addRef(this->_scripts, handle, 1);
}

void AssetManager::release(TextureHandle handle) {
    addRef(this->_textures, handle, -1);
}

void AssetManager::release(FontHandle handle) {
    addRef(this->_fonts, handle, -1);
}

void AssetManager::release(SoundHandle handle) {
    addRef(this->_sounds, handle, -1);
}

void AssetManager::release(MusicHandle handle) {
    addRef(this->_musics, handle, -1);
}

void AssetManager::release(ScriptHandle handle) {
    addRef(this->_scripts, handle, -1);
}

void AssetManager::collectUnused() {
    const auto sweep = [](Table& table, auto destroy) {
        std::erase_if(table.byPath, [&](const auto& item) {
            if (item.second.refs > 0) return false;
            destroy(item.second.id);
            table.byId.erase(item.second.id);
            return true;
        });
    };
    sweep(this->_textures, [this](uint32_t id) { this->_renderer->destroyTexture(TextureHandle{id}); });
    sweep(this->_fonts, [this](uint32_t id) { this->_renderer->destroyFont(FontHandle{id}); });
    sweep(this->_sounds, [this](uint32_t id) { this->_audio->destroySound(SoundHandle{id}); });
    sweep(this->_musics, [this](uint32_t id) { this->_audio->closeMusic(MusicHandle{id}); });
    sweep(this->_scripts, [this](uint32_t id) { this->_scripting->unload(ScriptHandle{id}); });
}

void AssetManager::reloadChanged() {
    std::error_code error;
    for (auto& [path, entry] : this->_scripts.byPath) {
        const auto time = std::filesystem::last_write_time(this->resolve(path), error);
        if (error || time <= entry.time) continue;
        entry.time = time;
        if (const auto reloaded = this->_scripting->reload(ScriptHandle{entry.id}); !reloaded)
            this->_log.error("cannot reload script '{}': {}", path, reloaded.error().toString());
        else
            this->_log.info("reloaded script '{}'", path);
    }
    const auto notice = [this, &error](Table& table, std::string_view kind) {
        for (auto& [path, entry] : table.byPath) {
            const auto time = std::filesystem::last_write_time(this->resolve(path), error);
            if (error || time <= entry.time) continue;
            entry.time = time;
            this->_log.info("{} '{}' changed on disk: restart to see it", kind, path);
        }
    };
    notice(this->_textures, "texture");
    notice(this->_fonts, "font");
    notice(this->_sounds, "sound");
}

AssetStats AssetManager::stats() const {
    AssetStats stats;
    stats.textures = static_cast<uint32_t>(this->_textures.byPath.size());
    stats.fonts = static_cast<uint32_t>(this->_fonts.byPath.size());
    stats.sounds = static_cast<uint32_t>(this->_sounds.byPath.size());
    stats.musics = static_cast<uint32_t>(this->_musics.byPath.size());
    stats.scripts = static_cast<uint32_t>(this->_scripts.byPath.size());
    for (const Table* table : {&this->_textures, &this->_fonts, &this->_sounds})
        for (const auto& [path, entry] : table->byPath)
            stats.bytes += entry.bytes;
    stats.failed = static_cast<uint32_t>(this->_failed.size());
    return stats;
}

}
