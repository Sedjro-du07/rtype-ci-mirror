/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** LogSinks
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/services/Logger.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

namespace engine {

ENGINE_API std::string formatRecord(const LogRecord& record);

class ENGINE_API ConsoleSink final : public ILogSink {
private:
    bool _color;

public:
    explicit ConsoleSink(bool color);
    void write(const LogRecord& record) override;
    void flush() override;
};

class ENGINE_API FileSink final : public ILogSink {
private:
    std::filesystem::path _file;
    size_t _maxBytes;
    int _keepFiles;
    std::ofstream _out;
    size_t _written = 0;

    void open();
    void rotate();

public:
    FileSink(std::filesystem::path file, size_t maxBytes, int keepFiles);
    bool isOpen() const;
    void write(const LogRecord& record) override;
    void flush() override;
};

}
