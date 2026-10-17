/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** DynamicLibrary
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Error.hpp"

#include <filesystem>
#include <string>
#include <string_view>

namespace engine {

class ENGINE_API DynamicLibrary {
private:
    void* _handle = nullptr;
    std::filesystem::path _path;

    DynamicLibrary(void* handle, std::filesystem::path path);
    void close() noexcept;

public:
    static Result<DynamicLibrary> open(const std::filesystem::path& file);
    static std::string decorate(std::string_view stem);

    DynamicLibrary(const DynamicLibrary&) = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;
    DynamicLibrary(DynamicLibrary&& other) noexcept;
    DynamicLibrary& operator=(DynamicLibrary&& other) noexcept;
    ~DynamicLibrary();

    void* rawSymbol(std::string_view name) const;
    template <class Fn>
    Result<Fn> symbol(std::string_view name) const;
    const std::filesystem::path& path() const;
};

}

#include "engine/module/DynamicLibrary.inl"
