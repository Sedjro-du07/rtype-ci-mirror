/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Scheduler
*/

#include "engine/scheduler/Scheduler.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <format>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

namespace engine {

namespace {

uint8_t maskOf(Role role) {
    switch (role) {
    case Role::Server:
        return ServerRole;
    case Role::Editor:
        return EditorRole;
    case Role::Client:
        break;
    }
    return ClientRole;
}

std::string_view stageName(Stage stage) {
    switch (stage) {
    case Stage::LateUpdate:
        return "LateUpdate";
    case Stage::Render:
        return "Render";
    case Stage::FixedUpdate:
        break;
    }
    return "FixedUpdate";
}

std::string describeCycle(const std::vector<std::vector<size_t>>& edges, const std::vector<bool>& left,
                          const std::vector<std::string_view>& names) {
    std::vector<int> state(names.size(), 0);
    std::vector<size_t> path;
    std::function<bool(size_t)> visit = [&](size_t node) {
        state[node] = 1;
        path.push_back(node);
        for (size_t next : edges[node]) {
            if (!left[next]) continue;
            if (state[next] == 1) {
                path.push_back(next);
                return true;
            }
            if (state[next] == 0 && visit(next)) return true;
        }
        state[node] = 2;
        path.pop_back();
        return false;
    };
    for (size_t start = 0; start < names.size(); ++start) {
        if (!left[start] || state[start] != 0 || !visit(start)) continue;
        const size_t loop = path.back();
        std::string text;
        bool inside = false;
        for (size_t node : path) {
            if (node == loop) inside = true;
            if (!inside) continue;
            if (!text.empty()) text += " -> ";
            text += names[node];
        }
        return text;
    }
    return {};
}

}

SystemBuilder::SystemBuilder(Scheduler& scheduler, size_t index) : _scheduler(&scheduler), _index(index) {}

SystemBuilder& SystemBuilder::after(std::string_view other) {
    this->_scheduler->_entries[this->_index].after.emplace_back(other);
    return *this;
}

SystemBuilder& SystemBuilder::before(std::string_view other) {
    this->_scheduler->_entries[this->_index].before.emplace_back(other);
    return *this;
}

SystemBuilder& SystemBuilder::roles(uint8_t mask) {
    this->_scheduler->_entries[this->_index].roles = mask;
    return *this;
}

SystemBuilder& SystemBuilder::disabled() {
    this->_scheduler->_entries[this->_index].startDisabled = true;
    return *this;
}

SystemBuilder& SystemBuilder::critical() {
    this->_scheduler->_entries[this->_index].critical = true;
    return *this;
}

Scheduler::Scheduler(Role role, Logger& log) : _role(role), _log(log.tagged("scheduler")) {}

bool Scheduler::included(const Entry& entry) const {
    return (entry.roles & maskOf(this->_role)) != 0;
}

Scheduler::Entry* Scheduler::findEntry(std::string_view name) {
    const auto it = std::ranges::find(this->_entries, name, &Entry::name);
    return it == this->_entries.end() ? nullptr : &*it;
}

const Scheduler::Entry* Scheduler::findEntry(std::string_view name) const {
    const auto it = std::ranges::find(this->_entries, name, &Entry::name);
    return it == this->_entries.end() ? nullptr : &*it;
}

SystemStats* Scheduler::statsOf(std::string_view name) {
    const auto it = std::ranges::find(this->_stats, name, &SystemStats::name);
    return it == this->_stats.end() ? nullptr : &*it;
}

SystemBuilder Scheduler::addFactory(Stage stage, std::string name, SystemFactory make) {
    Entry entry;
    entry.stage = stage;
    entry.name = std::move(name);
    entry.make = std::move(make);
    this->_entries.push_back(std::move(entry));
    return SystemBuilder(*this, this->_entries.size() - 1);
}

std::vector<SystemFn> Scheduler::instantiate(Stage stage) const {
    std::vector<SystemFn> systems;
    for (size_t index : this->_order[static_cast<size_t>(stage)])
        systems.push_back(this->_entries[index].make());
    return systems;
}

Result<void> Scheduler::sortStage(Stage stage) {
    std::vector<size_t> nodes;
    std::unordered_map<std::string_view, size_t> local;
    for (size_t i = 0; i < this->_entries.size(); ++i)
        if (this->_entries[i].stage == stage && this->included(this->_entries[i])) {
            local.emplace(this->_entries[i].name, nodes.size());
            nodes.push_back(i);
        }

    std::vector<std::vector<size_t>> edges(nodes.size());
    std::vector<size_t> incoming(nodes.size(), 0);
    const auto link = [&](const Entry& from, std::string_view other, bool otherFirst) -> Result<void> {
        const Entry* target = this->findEntry(other);
        if (!target)
            return fail(Errc::ScheduleCycle, std::format("system '{}' is ordered {} '{}', which is not registered",
                                                         from.name, otherFirst ? "after" : "before", other));
        if (target->stage != stage)
            return fail(
                Errc::ScheduleCycle,
                std::format("system '{}' ({}) cannot be ordered against '{}' ({}): stages are ordered by the loop",
                            from.name, stageName(stage), other, stageName(target->stage)));
        if (!this->included(*target)) return {};
        const size_t self = local.at(from.name);
        const size_t peer = local.at(target->name);
        const size_t first = otherFirst ? peer : self;
        const size_t second = otherFirst ? self : peer;
        edges[first].push_back(second);
        ++incoming[second];
        return {};
    };
    for (size_t node : nodes) {
        const Entry& entry = this->_entries[node];
        for (const std::string& other : entry.after)
            if (auto ok = link(entry, other, true); !ok) return ok;
        for (const std::string& other : entry.before)
            if (auto ok = link(entry, other, false); !ok) return ok;
    }

    std::priority_queue<size_t, std::vector<size_t>, std::greater<>> ready;
    for (size_t i = 0; i < nodes.size(); ++i)
        if (incoming[i] == 0) ready.push(i);
    std::vector<size_t>& order = this->_order[static_cast<size_t>(stage)];
    order.clear();
    std::vector<bool> left(nodes.size(), true);
    while (!ready.empty()) {
        const size_t next = ready.top();
        ready.pop();
        left[next] = false;
        order.push_back(nodes[next]);
        for (size_t after : edges[next])
            if (--incoming[after] == 0) ready.push(after);
    }
    if (order.size() == nodes.size()) return {};

    std::vector<std::string_view> names;
    for (size_t node : nodes)
        names.push_back(this->_entries[node].name);
    order.clear();
    return fail(Errc::ScheduleCycle,
                std::format("systems of {} form a cycle: {}", stageName(stage), describeCycle(edges, left, names)));
}

Result<void> Scheduler::build() {
    for (size_t i = 0; i < this->_entries.size(); ++i)
        for (size_t j = i + 1; j < this->_entries.size(); ++j)
            if (this->_entries[i].name == this->_entries[j].name)
                return fail(Errc::DuplicateName,
                            std::format("system '{}' is registered twice", this->_entries[i].name));
    for (size_t stage = 0; stage < StageCount; ++stage)
        if (auto ok = this->sortStage(static_cast<Stage>(stage)); !ok) return ok;

    this->_stats.clear();
    for (size_t stage = 0; stage < StageCount; ++stage)
        for (size_t index : this->_order[stage]) {
            Entry& entry = this->_entries[index];
            if (!entry.fn) entry.fn = entry.make();
            entry.stats = this->_stats.size();
            this->_stats.push_back(SystemStats{entry.name, entry.stage, 0, 0, !entry.startDisabled, false, {}});
        }
    return {};
}

void Scheduler::fault(Entry& entry, SystemStats& stats, std::string message) {
    stats.faulted = true;
    stats.enabled = false;
    stats.lastError = std::move(message);
    this->_log.error("system '{}' failed: {}", entry.name, stats.lastError);
    if (this->_role == Role::Server && !entry.critical) {
        this->_log.warn("system '{}' is disabled, the game continues", entry.name);
        return;
    }
    if (this->_role == Role::Server) this->_log.error("system '{}' is critical: the session ends", entry.name);
    this->_quit = true;
    this->_exitCode = 5;
}

void Scheduler::run(Stage stage, Context& ctx) {
    for (size_t index : this->_order[static_cast<size_t>(stage)]) {
        Entry& entry = this->_entries[index];
        SystemStats& stats = this->_stats[entry.stats];
        if (!stats.enabled || stats.faulted) continue;
        const auto start = std::chrono::steady_clock::now();
        try {
            entry.fn(ctx);
        } catch (const std::exception& error) {
            this->fault(entry, stats, error.what());
        } catch (...) {
            this->fault(entry, stats, "unknown exception");
        }
        const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        stats.lastMs = ms;
        stats.averageMs = stats.averageMs == 0 ? ms : stats.averageMs * 0.9 + ms * 0.1;
        if (this->_quit) return;
    }
}

void Scheduler::setEnabled(std::string_view name, bool enabled) {
    if (Entry* entry = this->findEntry(name)) entry->startDisabled = !enabled;
    if (SystemStats* stats = this->statsOf(name)) {
        stats->enabled = enabled;
        if (enabled) stats->faulted = false;
    }
}

bool Scheduler::enabled(std::string_view name) const {
    const auto it = std::ranges::find(this->_stats, name, &SystemStats::name);
    if (it != this->_stats.end()) return it->enabled;
    const Entry* entry = this->findEntry(name);
    return entry && !entry->startDisabled && this->included(*entry);
}

std::span<const SystemStats> Scheduler::stats() const {
    return this->_stats;
}

void Scheduler::beginGameSystems() {
    this->_engineCount = this->_entries.size();
}

void Scheduler::clearGameSystems() {
    this->_entries.erase(this->_entries.begin() + static_cast<std::ptrdiff_t>(this->_engineCount),
                         this->_entries.end());
    for (auto& order : this->_order)
        order.clear();
    this->_stats.clear();
}

bool Scheduler::quitRequested() const {
    return this->_quit;
}

int Scheduler::exitCode() const {
    return this->_exitCode;
}

}
