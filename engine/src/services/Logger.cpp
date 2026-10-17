/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Logger
*/

#include "engine/services/Logger.hpp"

#include "LogSinks.hpp"

#include <array>
#include <chrono>
#include <cstdio>
#include <system_error>
#include <utility>

namespace engine {

namespace {

constexpr std::array<std::string_view, 5> LevelNames{"TRACE", "DEBUG", "INFO ", "WARN ", "ERROR"};
constexpr std::array<std::string_view, 5> LevelColors{"\033[90m", "\033[36m", "\033[0m", "\033[33m", "\033[31m"};

std::string_view levelName(LogLevel level) {
    return LevelNames[static_cast<size_t>(level)];
}

}

ILogSink::~ILogSink() = default;

void ILogSink::flush() {}

std::string formatRecord(const LogRecord& record) {
    std::string line = std::format("{:%F %T} {} ", std::chrono::floor<std::chrono::milliseconds>(record.time),
                                   levelName(record.level));
    if (!record.tag.empty()) line += std::format("[{}] ", record.tag);
    line += record.message;
    return line;
}

ConsoleSink::ConsoleSink(bool color) : _color(color) {}

void ConsoleSink::write(const LogRecord& record) {
    std::FILE* stream = record.level >= LogLevel::Warn ? stderr : stdout;
    const std::string line = formatRecord(record);
    if (this->_color)
        std::fprintf(stream, "%s%s\033[0m\n", LevelColors[static_cast<size_t>(record.level)].data(), line.c_str());
    else
        std::fprintf(stream, "%s\n", line.c_str());
}

void ConsoleSink::flush() {
    std::fflush(stdout);
    std::fflush(stderr);
}

FileSink::FileSink(std::filesystem::path file, size_t maxBytes, int keepFiles)
    : _file(std::move(file)), _maxBytes(maxBytes), _keepFiles(keepFiles) {
    this->open();
}

void FileSink::open() {
    std::error_code error;
    if (this->_file.has_parent_path()) std::filesystem::create_directories(this->_file.parent_path(), error);
    this->_out.open(this->_file, std::ios::app);
    this->_written = std::filesystem::exists(this->_file, error) ? std::filesystem::file_size(this->_file, error) : 0;
}

void FileSink::rotate() {
    this->_out.close();
    std::error_code error;
    const auto numbered = [this](int i) {
        std::filesystem::path p = this->_file;
        p.replace_extension(std::format(".{}{}", i, this->_file.extension().string()));
        return p;
    };
    std::filesystem::remove(numbered(this->_keepFiles), error);
    for (int i = this->_keepFiles - 1; i >= 1; --i)
        std::filesystem::rename(numbered(i), numbered(i + 1), error);
    if (this->_keepFiles >= 1) std::filesystem::rename(this->_file, numbered(1), error);
    std::filesystem::remove(this->_file, error);
    this->_out.open(this->_file, std::ios::trunc);
    this->_written = 0;
}

bool FileSink::isOpen() const {
    return this->_out.is_open();
}

void FileSink::write(const LogRecord& record) {
    if (!this->_out.is_open()) return;
    const std::string line = formatRecord(record) + '\n';
    if (this->_written > 0 && this->_written + line.size() > this->_maxBytes) this->rotate();
    this->_out << line;
    this->_written += line.size();
}

void FileSink::flush() {
    this->_out.flush();
}

Logger::Logger(std::shared_ptr<Shared> shared, std::string tag) : _shared(std::move(shared)), _tag(std::move(tag)) {}

Logger::Logger(const LogConfig& config) : _shared(std::make_shared<Shared>()) {
    this->_shared->level = config.level;
    this->_shared->sinks.push_back(std::make_shared<ConsoleSink>(config.color));
    if (!config.file.empty()) {
        auto file = std::make_shared<FileSink>(config.file, 4u * 1024u * 1024u, 3);
        if (file->isOpen())
            this->_shared->sinks.push_back(std::move(file));
        else
            this->warn("cannot open the log file '{}': logging to the console only", config.file.string());
    }
}

bool Logger::enabled(LogLevel level) const {
    return level >= this->_shared->level.load(std::memory_order_relaxed);
}

void Logger::write(LogLevel level, std::string_view message) {
    const LogRecord record{level, this->_tag, message, std::chrono::system_clock::now()};
    const std::lock_guard lock(this->_shared->mutex);
    for (const auto& sink : this->_shared->sinks)
        sink->write(record);
    if (level == LogLevel::Error)
        for (const auto& sink : this->_shared->sinks)
            sink->flush();
}

Logger Logger::tagged(std::string_view tag) const {
    return Logger(this->_shared, std::string(tag));
}

void Logger::setLevel(LogLevel level) {
    this->_shared->level = level;
}

LogLevel Logger::level() const {
    return this->_shared->level.load();
}

void Logger::addSink(std::shared_ptr<ILogSink> sink) {
    const std::lock_guard lock(this->_shared->mutex);
    this->_shared->sinks.push_back(std::move(sink));
}

void Logger::flush() {
    const std::lock_guard lock(this->_shared->mutex);
    for (const auto& sink : this->_shared->sinks)
        sink->flush();
}

}
