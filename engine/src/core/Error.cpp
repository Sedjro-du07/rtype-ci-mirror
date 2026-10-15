/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Error
*/

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
