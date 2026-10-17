/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** AssetManager
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/render/Handles.hpp"
#include "engine/services/Logger.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

namespace engine {

class IRenderer;
class IAudio;
class IScripting;

struct AssetStats {
    uint32_t textures = 0, fonts = 0, sounds = 0, musics = 0, scripts = 0;
    uint64_t bytes = 0;
    uint32_t failed = 0;
};

class ENGINE_API AssetManager {
private:
    struct Entry {
        uint32_t id = 0;
        uint32_t refs = 0;
        uint64_t bytes = 0;
        std::filesystem::file_time_type time{};
    };

    struct Table {
        std::unordered_map<std::string, Entry> byPath;
        std::unordered_map<uint32_t, std::string> byId;
    };

    Logger _log;
    std::filesystem::path _root;
    IRenderer* _renderer = nullptr;
    IAudio* _audio = nullptr;
    IScripting* _scripting = nullptr;
    Table _textures, _fonts, _sounds, _musics, _scripts;
    std::unordered_map<std::string, uint32_t> _failed;
    TextureHandle _missing;

    TextureHandle missingTexture();
    bool readFile(std::string_view path, std::filesystem::path& resolved, std::string& bytes);
    void fail(std::string_view kind, std::string_view path, std::string_view reason);

public:
    AssetManager(Logger& log, std::filesystem::path root);
    void attach(IRenderer& renderer, IAudio& audio, IScripting& scripting);

    TextureHandle texture(std::string_view path);
    FontHandle font(std::string_view path);
    SoundHandle sound(std::string_view path);
    MusicHandle music(std::string_view path);
    ScriptHandle script(std::string_view path);

    std::string_view pathOf(TextureHandle handle) const;
    std::string_view pathOf(FontHandle handle) const;
    std::string_view pathOf(SoundHandle handle) const;
    std::string_view pathOf(MusicHandle handle) const;
    std::string_view pathOf(ScriptHandle handle) const;
    void retain(TextureHandle handle);
    void retain(FontHandle handle);
    void retain(SoundHandle handle);
    void retain(MusicHandle handle);
    void retain(ScriptHandle handle);
    void release(TextureHandle handle);
    void release(FontHandle handle);
    void release(SoundHandle handle);
    void release(MusicHandle handle);
    void release(ScriptHandle handle);
    void collectUnused();
    void reloadChanged();
    std::filesystem::path resolve(std::string_view path) const;
    AssetStats stats() const;
};

}
