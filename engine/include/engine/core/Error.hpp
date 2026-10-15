/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Error
*/

#pragma once

#include "engine/Export.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

#if defined(ENGINE_USE_TL_EXPECTED)
#include <tl/expected.hpp>
#else
#include <expected>
#endif

namespace engine {

enum class Errc : uint16_t {
    ConfigInvalid,
    FileNotFound,
    ParseError,
    PluginNotFound,
    SymbolNotFound,
    ApiMismatch,
    GameModuleInvalid,
    UnknownComponent,
    UnknownPrefab,
    DuplicateName,
    ScheduleCycle,
    AssetLoadFailed,
    ScriptError,
    NetworkError,
    InvalidArgument
};

struct ENGINE_API Error {
    Errc code = Errc::InvalidArgument;
    std::string message;
    std::string file;
    int line = 0;

    std::string toString() const;
};

#if defined(ENGINE_USE_TL_EXPECTED)
template <class T, class E>
using Expected = tl::expected<T, E>;
using tl::unexpected;
#else
template <class T, class E>
using Expected = std::expected<T, E>;
using std::unexpected;
#endif

template <class T>
using Result = Expected<T, Error>;

inline unexpected<Error> fail(Errc code, std::string message, std::string file = {}, int line = 0) {
    return unexpected<Error>(Error{code, std::move(message), std::move(file), line});
}

class ENGINE_API BootError : public std::runtime_error {
private:
    int exitCode_;
    Error error_;

public:
    BootError(int exitCode, Error error);
    int exitCode() const noexcept { return exitCode_; }
    const Error& error() const noexcept { return error_; }
};

}
