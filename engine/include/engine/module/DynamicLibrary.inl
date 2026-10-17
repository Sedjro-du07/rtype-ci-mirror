/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** DynamicLibrary
*/

#pragma once

#include "engine/module/DynamicLibrary.hpp"

#include <cstring>
#include <string>
#include <type_traits>

namespace engine {

template <class Fn>
Result<Fn> DynamicLibrary::symbol(std::string_view name) const {
    static_assert(std::is_pointer_v<Fn> && std::is_function_v<std::remove_pointer_t<Fn>>,
                  "DynamicLibrary::symbol<Fn>: Fn is a function pointer type");
    void* raw = this->rawSymbol(name);
    if (!raw)
        return fail(Errc::SymbolNotFound, "symbol '" + std::string(name) + "' not found", this->_path.generic_string());
    Fn fn = nullptr;
    std::memcpy(&fn, &raw, sizeof(fn));
    return fn;
}

}
