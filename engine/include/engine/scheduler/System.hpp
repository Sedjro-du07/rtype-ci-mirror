/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** System
*/

#pragma once

#include <cstdint>
#include <functional>

namespace engine {

class Context;

using SystemFn = std::move_only_function<void(Context&)>;

enum RoleMask : uint8_t { ClientRole = 1, ServerRole = 2, EditorRole = 4, AllRoles = 7 };

}
