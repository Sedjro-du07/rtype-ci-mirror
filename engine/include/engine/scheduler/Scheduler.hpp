/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Scheduler
*/

#pragma once

#include "engine/EngineConfig.hpp"
#include "engine/Export.hpp"
#include "engine/core/Error.hpp"
#include "engine/scheduler/Stage.hpp"
#include "engine/scheduler/System.hpp"
#include "engine/services/Logger.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace engine {

class Scheduler;

struct SystemStats {
    std::string_view name;
    Stage stage = Stage::FixedUpdate;
    double lastMs = 0, averageMs = 0;
    bool enabled = true;
    bool faulted = false;
    std::string lastError;
};

class ENGINE_API SystemBuilder {
private:
    Scheduler* _scheduler;
    size_t _index;

public:
    SystemBuilder(Scheduler& scheduler, size_t index);

    SystemBuilder& after(std::string_view other);
    SystemBuilder& before(std::string_view other);
    SystemBuilder& roles(uint8_t mask);
    SystemBuilder& disabled();
    SystemBuilder& critical();
};

class ENGINE_API Scheduler {
private:
    struct Entry {
        Stage stage = Stage::FixedUpdate;
        std::string name;
        std::function<SystemFn()> make;
        SystemFn fn;
        std::vector<std::string> after, before;
        uint8_t roles = AllRoles;
        bool critical = false;
        bool startDisabled = false;
        size_t stats = 0;
    };

    Role _role;
    Logger _log;
    std::vector<Entry> _entries;
    std::array<std::vector<size_t>, StageCount> _order;
    std::vector<SystemStats> _stats;
    size_t _engineCount = 0;
    bool _quit = false;
    int _exitCode = 0;

    friend class SystemBuilder;

    bool included(const Entry& entry) const;
    Entry* findEntry(std::string_view name);
    const Entry* findEntry(std::string_view name) const;
    SystemStats* statsOf(std::string_view name);
    Result<void> sortStage(Stage stage);
    void fault(Entry& entry, SystemStats& stats, std::string message);

public:
    using SystemFactory = std::function<SystemFn()>;

    Scheduler(Role role, Logger& log);
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    template <class F>
    SystemBuilder add(Stage stage, std::string name, F fn);
    SystemBuilder addFactory(Stage stage, std::string name, SystemFactory make);
    std::vector<SystemFn> instantiate(Stage stage) const;

    Result<void> build();
    void run(Stage stage, Context& ctx);
    void setEnabled(std::string_view name, bool enabled);
    bool enabled(std::string_view name) const;
    std::span<const SystemStats> stats() const;
    void beginGameSystems();
    void clearGameSystems();
    bool quitRequested() const;
    int exitCode() const;
};

}

#include "engine/scheduler/Scheduler.inl"
