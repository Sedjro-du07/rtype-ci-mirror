/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Logger
*/

#pragma once

#include "engine/Export.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace engine {

enum class LogLevel : uint8_t { Trace, Debug, Info, Warn, Error };

struct LogRecord {
    LogLevel level = LogLevel::Info;
    std::string_view tag;
    std::string_view message;
    std::chrono::system_clock::time_point time;
};

class ENGINE_API ILogSink {
public:
    virtual ~ILogSink();
    virtual void write(const LogRecord& record) = 0;
    virtual void flush();
};

struct LogConfig {
    LogLevel level = LogLevel::Info;
    std::filesystem::path file = "logs/engine.log";
    bool color = true;
};

class ENGINE_API Logger {
private:
    struct Shared {
        std::mutex mutex;
        std::vector<std::shared_ptr<ILogSink>> sinks;
        std::atomic<LogLevel> level{LogLevel::Info};
    };

    std::shared_ptr<Shared> _shared;
    std::string _tag;

    Logger(std::shared_ptr<Shared> shared, std::string tag);
    bool enabled(LogLevel level) const;
    void write(LogLevel level, std::string_view message);

    template <class... A>
    void log(LogLevel level, std::format_string<A...> format, A&&... args);

public:
    explicit Logger(const LogConfig& config);

    template <class... A>
    void trace(std::format_string<A...> format, A&&... args);
    template <class... A>
    void debug(std::format_string<A...> format, A&&... args);
    template <class... A>
    void info(std::format_string<A...> format, A&&... args);
    template <class... A>
    void warn(std::format_string<A...> format, A&&... args);
    template <class... A>
    void error(std::format_string<A...> format, A&&... args);

    Logger tagged(std::string_view tag) const;
    void setLevel(LogLevel level);
    LogLevel level() const;
    void addSink(std::shared_ptr<ILogSink> sink);
    void flush();
};

}

#include "engine/services/Logger.inl"
