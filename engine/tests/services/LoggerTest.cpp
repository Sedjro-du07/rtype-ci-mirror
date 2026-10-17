/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** LoggerTest
*/

#include "engine/services/Logger.hpp"

#include "services/LogSinks.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace engine;

namespace {

class MemorySink final : public ILogSink {
public:
    std::vector<std::string> lines;
    std::vector<LogLevel> levels;
    int flushes = 0;

    void write(const LogRecord& record) override {
        this->lines.push_back(formatRecord(record));
        this->levels.push_back(record.level);
    }
    void flush() override { ++this->flushes; }
};

LogConfig quietConfig() {
    LogConfig config;
    config.file.clear();
    config.level = LogLevel::Trace;
    return config;
}

std::filesystem::path freshFolder(const std::string& name) {
    const auto folder = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(folder);
    return folder;
}

}

TEST_CASE("levels below the threshold are not written", "[logger]") {
    LogConfig config = quietConfig();
    config.level = LogLevel::Warn;
    Logger log(config);
    auto sink = std::make_shared<MemorySink>();
    log.addSink(sink);

    log.info("hidden {}", 1);
    log.warn("shown {}", 2);
    log.error("shown {}", 3);
    REQUIRE(sink->lines.size() == 2);
    CHECK(sink->levels == std::vector<LogLevel>{LogLevel::Warn, LogLevel::Error});
    CHECK(sink->lines[0].ends_with("WARN  shown 2"));
    CHECK(sink->flushes == 1);

    log.setLevel(LogLevel::Debug);
    log.debug("now visible");
    CHECK(sink->lines.size() == 3);
}

TEST_CASE("a tagged logger shares the sinks and prefixes its tag", "[logger]") {
    Logger log(quietConfig());
    auto sink = std::make_shared<MemorySink>();
    log.addSink(sink);
    Logger sfml = log.tagged("platform_sfml");
    sfml.info("window {}x{}", 1280, 720);
    log.info("untagged");
    REQUIRE(sink->lines.size() == 2);
    CHECK(sink->lines[0].ends_with("INFO  [platform_sfml] window 1280x720"));
    CHECK(sink->lines[1].ends_with("INFO  untagged"));
    sfml.setLevel(LogLevel::Error);
    CHECK(log.level() == LogLevel::Error);
}

TEST_CASE("lines from several threads never interleave", "[logger]") {
    Logger log(quietConfig());
    auto sink = std::make_shared<MemorySink>();
    log.addSink(sink);
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t)
        threads.emplace_back([&log, t] {
            Logger mine = log.tagged("thread" + std::to_string(t));
            for (int i = 0; i < 500; ++i)
                mine.info("message {} from {}", i, t);
        });
    for (std::thread& thread : threads)
        thread.join();
    REQUIRE(sink->lines.size() == 2000);
    for (const std::string& line : sink->lines) {
        const auto tag = line.find("[thread");
        REQUIRE(tag != std::string::npos);
        const char t = line[tag + 7];
        CHECK(line.ends_with(std::string(" from ") + t));
    }
}

TEST_CASE("the file sink writes lines and rotates", "[logger]") {
    const auto folder = freshFolder("rtype-logger-test");
    const auto file = folder / "engine.log";
    {
        FileSink sink(file, 200, 2);
        REQUIRE(sink.isOpen());
        for (int i = 0; i < 20; ++i)
            sink.write(LogRecord{LogLevel::Info, "test", "a line of about forty characters", {}});
        sink.flush();
    }
    CHECK(std::filesystem::exists(file));
    CHECK(std::filesystem::exists(folder / "engine.1.log"));
    CHECK(std::filesystem::exists(folder / "engine.2.log"));
    CHECK_FALSE(std::filesystem::exists(folder / "engine.3.log"));
    CHECK(std::filesystem::file_size(file) <= 200);
    std::ifstream in(file);
    std::string first;
    std::getline(in, first);
    CHECK(first.ends_with("[test] a line of about forty characters"));
    std::filesystem::remove_all(folder);
}

TEST_CASE("the logger opens its file from the configuration", "[logger]") {
    const auto folder = freshFolder("rtype-logger-config");
    LogConfig config = quietConfig();
    config.file = folder / "logs" / "client.log";
    config.color = false;
    {
        Logger log(config);
        log.info("started");
        log.flush();
    }
    std::ifstream in(config.file);
    std::string line;
    std::getline(in, line);
    CHECK(line.ends_with("INFO  started"));
    std::filesystem::remove_all(folder);
}
