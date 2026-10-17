/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Handles
*/

#pragma once

#include <cstdint>

namespace engine {

template <class Tag>
struct Handle {
    uint32_t id = 0;

    constexpr bool valid() const noexcept;
    friend constexpr bool operator==(Handle, Handle) = default;
};

using TextureHandle = Handle<struct TextureTag>;
using FontHandle = Handle<struct FontTag>;
using SoundHandle = Handle<struct SoundTag>;
using MusicHandle = Handle<struct MusicTag>;
using ScriptHandle = Handle<struct ScriptTag>;

}

#include "engine/render/Handles.inl"
