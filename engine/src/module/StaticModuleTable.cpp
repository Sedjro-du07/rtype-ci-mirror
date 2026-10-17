/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** StaticModuleTable
*/

#include "engine/module/StaticModuleTable.hpp"

#include <algorithm>
#include <deque>

namespace engine {

namespace {

std::deque<PluginEntry>& plugins() {
    static std::deque<PluginEntry> table;
    return table;
}

std::deque<GameEntry>& games() {
    static std::deque<GameEntry> table;
    return table;
}

}

void StaticModuleTable::addPlugin(std::string_view stem, CreatePlugin create, DestroyPlugin destroy) {
    plugins().push_back(PluginEntry{stem, create, destroy});
}

void StaticModuleTable::addGame(std::string_view name, CreateGame create, DestroyGame destroy) {
    games().push_back(GameEntry{name, create, destroy});
}

const PluginEntry* StaticModuleTable::findPlugin(std::string_view stem) {
    const auto it = std::ranges::find(plugins(), stem, &PluginEntry::stem);
    return it == plugins().end() ? nullptr : &*it;
}

const GameEntry* StaticModuleTable::findGame(std::string_view name) {
    const auto it = std::ranges::find(games(), name, &GameEntry::name);
    return it == games().end() ? nullptr : &*it;
}

}
