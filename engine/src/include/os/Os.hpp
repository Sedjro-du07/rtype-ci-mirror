/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Os
*/

#pragma once

#include "engine/Export.hpp"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <span>
#include <string_view>

namespace engine::os {

ENGINE_API void secureRandom(std::span<std::byte> out);
ENGINE_API std::filesystem::path userDataDir(std::string_view app);
ENGINE_API std::filesystem::path executableDir();
ENGINE_API void onQuitSignal(std::move_only_function<void()> fn);

}
