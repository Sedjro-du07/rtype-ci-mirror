/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** DynamicLibrary
*/

#include "engine/module/DynamicLibrary.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <string>
#include <utility>

namespace engine {

namespace {

#if defined(_WIN32)

std::string lastError() {
    const DWORD code = GetLastError();
    wchar_t* text = nullptr;
    const DWORD length =
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr, code, 0, reinterpret_cast<wchar_t*>(&text), 0, nullptr);
    std::wstring message(text ? text : L"", length);
    if (text) LocalFree(text);
    while (!message.empty() && (message.back() == L'\n' || message.back() == L'\r' || message.back() == L' '))
        message.pop_back();
    const int size =
        WideCharToMultiByte(CP_UTF8, 0, message.data(), static_cast<int>(message.size()), nullptr, 0, nullptr, nullptr);
    std::string utf8(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, message.data(), static_cast<int>(message.size()), utf8.data(), size, nullptr,
                        nullptr);
    return utf8.empty() ? "error " + std::to_string(code) : utf8;
}

void* openLibrary(const std::filesystem::path& file) {
    return reinterpret_cast<void*>(LoadLibraryW(file.wstring().c_str()));
}

void* findSymbol(void* handle, const std::string& name) {
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle), name.c_str()));
}

void closeLibrary(void* handle) {
    FreeLibrary(static_cast<HMODULE>(handle));
}

#else

std::string lastError() {
    const char* text = dlerror();
    return text ? text : "unknown error";
}

void* openLibrary(const std::filesystem::path& file) {
    return dlopen(file.c_str(), RTLD_NOW | RTLD_LOCAL);
}

void* findSymbol(void* handle, const std::string& name) {
    return dlsym(handle, name.c_str());
}

void closeLibrary(void* handle) {
    dlclose(handle);
}

#endif

}

DynamicLibrary::DynamicLibrary(void* handle, std::filesystem::path path) : _handle(handle), _path(std::move(path)) {}

Result<DynamicLibrary> DynamicLibrary::open(const std::filesystem::path& file) {
    std::error_code error;
    if (!std::filesystem::is_regular_file(file, error))
        return fail(Errc::FileNotFound, "library not found", file.generic_string());
    void* handle = openLibrary(file);
    if (!handle) return fail(Errc::FileNotFound, "cannot load the library: " + lastError(), file.generic_string());
    return DynamicLibrary(handle, file);
}

std::string DynamicLibrary::decorate(std::string_view stem) {
#if defined(_WIN32)
    return "lib" + std::string(stem) + ".dll";
#elif defined(__APPLE__)
    return "lib" + std::string(stem) + ".dylib";
#else
    return "lib" + std::string(stem) + ".so";
#endif
}

DynamicLibrary::DynamicLibrary(DynamicLibrary&& other) noexcept
    : _handle(std::exchange(other._handle, nullptr)), _path(std::move(other._path)) {}

DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other) noexcept {
    if (this != &other) {
        this->close();
        this->_handle = std::exchange(other._handle, nullptr);
        this->_path = std::move(other._path);
    }
    return *this;
}

DynamicLibrary::~DynamicLibrary() {
    this->close();
}

void DynamicLibrary::close() noexcept {
    if (this->_handle) closeLibrary(this->_handle);
    this->_handle = nullptr;
}

void* DynamicLibrary::rawSymbol(std::string_view name) const {
    return this->_handle ? findSymbol(this->_handle, std::string(name)) : nullptr;
}

const std::filesystem::path& DynamicLibrary::path() const {
    return this->_path;
}

}
