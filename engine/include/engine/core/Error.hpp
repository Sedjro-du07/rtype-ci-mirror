// Errors and results: an expected failure is returned, never thrown (ADR 0024).
// ENG-04 · ADR 0017, 0024 · docs/engine/reference/01-core.md

#pragma once

#include "engine/Export.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

#if defined(ENGINE_USE_TL_EXPECTED) // set by the C++23 probe when <expected> is missing (ENG-03)
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
    std::string message; // human readable, complete sentence
    std::string file;    // optional: source file of the error (YAML, script)
    int line = 0;        // optional: 1-based line in that file

    // "scenes/level1.yaml:14: unknown component 'Helth'", "client.yaml: …", or just the message.
    std::string toString() const;
};

// std::expected, or tl::expected when the compiler lacks it (same interface).
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

// return fail(Errc::FileNotFound, "cannot open 'client.yaml'");   (works with both expected types)
inline unexpected<Error> fail(Errc code, std::string message, std::string file = {}, int line = 0) {
    return unexpected<Error>(Error{code, std::move(message), std::move(file), line});
}

// The engine's only exception: thrown by Engine::boot() and load(), caught by run(), which returns
// exitCode(): 1 config, 2 plugin, 3 game module or schedule, 4 scene (ADR 0024).
class ENGINE_API BootError : public std::runtime_error {
public:
    BootError(int exitCode, Error error);
    int exitCode() const noexcept { return exitCode_; }
    const Error& error() const noexcept { return error_; }

private:
    int exitCode_;
    Error error_;
};

} // namespace engine
