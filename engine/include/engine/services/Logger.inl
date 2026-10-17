/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Logger
*/

#pragma once

#include "engine/services/Logger.hpp"

#include <format>
#include <utility>

namespace engine {

template <class... A>
void Logger::log(LogLevel level, std::format_string<A...> format, A&&... args) {
    if (!this->enabled(level)) return;
    this->write(level, std::format(format, std::forward<A>(args)...));
}

template <class... A>
void Logger::trace(std::format_string<A...> format, A&&... args) {
    this->log(LogLevel::Trace, format, std::forward<A>(args)...);
}

template <class... A>
void Logger::debug(std::format_string<A...> format, A&&... args) {
    this->log(LogLevel::Debug, format, std::forward<A>(args)...);
}

template <class... A>
void Logger::info(std::format_string<A...> format, A&&... args) {
    this->log(LogLevel::Info, format, std::forward<A>(args)...);
}

template <class... A>
void Logger::warn(std::format_string<A...> format, A&&... args) {
    this->log(LogLevel::Warn, format, std::forward<A>(args)...);
}

template <class... A>
void Logger::error(std::format_string<A...> format, A&&... args) {
    this->log(LogLevel::Error, format, std::forward<A>(args)...);
}

}
