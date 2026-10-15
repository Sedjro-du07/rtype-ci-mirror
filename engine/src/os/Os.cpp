/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Os
*/

#include "Os.hpp"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <system_error>
#include <utility>

#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <bcrypt.h>
#include <windows.h>
#else
#include <sys/random.h>

#include <cerrno>
#include <csignal>
#endif

namespace engine::os {

namespace {

std::move_only_function<void()> quitHandler;

std::filesystem::path createdOrAsIs(std::filesystem::path dir) {
    std::error_code error;
    std::filesystem::create_directories(dir, error);
    return dir;
}

#if defined(_WIN32)

std::filesystem::path environmentPath(const wchar_t* name) {
    const DWORD size = GetEnvironmentVariableW(name, nullptr, 0);
    if (size == 0) return {};
    std::wstring value(size, L'\0');
    const DWORD written = GetEnvironmentVariableW(name, value.data(), size);
    value.resize(written);
    return value;
}

BOOL WINAPI handleConsoleEvent(DWORD event) {
    switch (event) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        if (quitHandler) quitHandler();
        return TRUE;
    default:
        return FALSE;
    }
}

#else

void handleSignal(int) {
    if (quitHandler) quitHandler();
}

#endif

}

#if defined(_WIN32)

void secureRandom(std::span<std::byte> out) {
    while (!out.empty()) {
        const ULONG chunk = static_cast<ULONG>(std::min<size_t>(out.size(), 1u << 30));
        const NTSTATUS status =
            BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(out.data()), chunk, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        if (status < 0) std::abort();
        out = out.subspan(chunk);
    }
}

std::filesystem::path userDataDir(std::string_view app) {
    std::filesystem::path base = environmentPath(L"APPDATA");
    if (base.empty() || !base.is_absolute()) base = std::filesystem::temp_directory_path();
    return createdOrAsIs(base / std::filesystem::path(app));
}

std::filesystem::path executableDir() {
    std::wstring buffer(MAX_PATH, L'\0');
    for (;;) {
        const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0) return std::filesystem::current_path();
        if (length < buffer.size()) {
            buffer.resize(length);
            return std::filesystem::path(buffer).parent_path();
        }
        buffer.resize(buffer.size() * 2);
    }
}

void onQuitSignal(std::move_only_function<void()> fn) {
    quitHandler = std::move(fn);
    SetConsoleCtrlHandler(handleConsoleEvent, TRUE);
}

#else

void secureRandom(std::span<std::byte> out) {
    while (!out.empty()) {
        const ssize_t written = getrandom(out.data(), out.size(), 0);
        if (written < 0) {
            if (errno == EINTR) continue;
            std::abort();
        }
        out = out.subspan(static_cast<size_t>(written));
    }
}

std::filesystem::path userDataDir(std::string_view app) {
    std::filesystem::path base;
    if (const char* xdg = std::getenv("XDG_DATA_HOME"); xdg && std::filesystem::path(xdg).is_absolute())
        base = xdg;
    else if (const char* home = std::getenv("HOME"); home && *home)
        base = std::filesystem::path(home) / ".local" / "share";
    else
        base = std::filesystem::temp_directory_path();
    return createdOrAsIs(base / std::filesystem::path(app));
}

std::filesystem::path executableDir() {
    std::error_code error;
    const auto exe = std::filesystem::read_symlink("/proc/self/exe", error);
    return error ? std::filesystem::current_path() : exe.parent_path();
}

void onQuitSignal(std::move_only_function<void()> fn) {
    quitHandler = std::move(fn);
    struct sigaction action {};
    action.sa_handler = handleSignal;
    action.sa_flags = SA_RESTART;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGTERM, &action, nullptr);
}

#endif

}
