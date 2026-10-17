/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Context
*/

#include "engine/Context.hpp"

#include "engine/input/InputMap.hpp"
#include "engine/scene/SceneManager.hpp"

#include <cassert>
#include <utility>

namespace engine {

Context::Context(ServiceContext services, Parts parts)
    : ServiceContext(services), _scenes(&parts.scenes), _commands(&parts.commands), _events(&parts.events),
      _time(&parts.time), _input(&parts.input), _render(&parts.render), _audio(&parts.audio), _network(parts.network),
      _quit(std::move(parts.quit)) {}

Registry& Context::world() {
    return this->_scenes->world();
}

Commands& Context::commands() {
    return *this->_commands;
}

SceneManager& Context::scenes() {
    return *this->_scenes;
}

EventBus& Context::events() {
    return *this->_events;
}

const Time& Context::time() const {
    return *this->_time;
}

const ActionState& Context::input() const {
    return this->_input->state();
}

InputMap& Context::inputMap() {
    return *this->_input;
}

RenderQueue& Context::render() {
    return *this->_render;
}

IAudio& Context::audio() {
    return *this->_audio;
}

NetworkSession& Context::network() {
    assert(this->_network);
    return *this->_network;
}

bool Context::online() const {
    return this->_network != nullptr;
}

void Context::requestQuit() {
    if (this->_quit) this->_quit();
}

}
