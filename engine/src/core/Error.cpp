// ENG-04 · docs/engine/reference/01-core.md

#include "engine/core/Error.hpp"

namespace engine {

std::string Error::toString() const {
    if (file.empty()) return message;
    if (line > 0) return file + ":" + std::to_string(line) + ": " + message;
    return file + ": " + message;
}

BootError::BootError(int exitCode, Error error)
    : std::runtime_error(error.toString()), exitCode_(exitCode), error_(std::move(error)) {}

} // namespace engine
