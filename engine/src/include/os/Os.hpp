/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Os
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Error.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <span>
#include <string_view>
#include <vector>

namespace engine::os {

ENGINE_API void secureRandom(std::span<std::byte> out);
ENGINE_API std::filesystem::path userDataDir(std::string_view app);
ENGINE_API std::filesystem::path executableDir();
ENGINE_API void onQuitSignal(std::move_only_function<void()> fn);

struct HostAddress {
    std::array<uint8_t, 16> bytes{};
    bool v6 = false;
};

ENGINE_API Result<std::vector<HostAddress>> resolveHost(std::string_view host);

}
