/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EventBus
*/

#include "engine/events/EventBus.hpp"

#include <utility>

namespace engine {

Subscription EventBus::subscribeRaw(uint64_t key, Listener listener) {
    std::unique_ptr<Channel>& channel = this->_channels[key];
    if (!channel) channel = std::make_unique<Channel>();
    Signal<void(const void*)>& signal = this->_gamePhase ? channel->game : channel->engine;
    return signal.connect(std::move(listener));
}

void EventBus::deliver(uint64_t key, const void* event) {
    const auto it = this->_channels.find(key);
    if (it == this->_channels.end()) return;
    Channel& channel = *it->second;
    channel.engine.emit(event);
    channel.game.emit(event);
}

void EventBus::post(Delivery delivery) {
    this->_queue.push_back(std::move(delivery));
}

void EventBus::postFromAnyThread(Delivery delivery) {
    const std::lock_guard lock(this->_inboxMutex);
    this->_inbox.push_back(std::move(delivery));
}

void EventBus::dispatch() {
    std::vector<Delivery> pending;
    pending.swap(this->_queue);
    {
        const std::lock_guard lock(this->_inboxMutex);
        for (Delivery& delivery : this->_inbox)
            pending.push_back(std::move(delivery));
        this->_inbox.clear();
    }
    for (Delivery& delivery : pending)
        delivery();
}

void EventBus::beginGameSubscriptions() {
    this->_gamePhase = true;
}

void EventBus::clearGameSubscriptions() {
    for (auto& [key, channel] : this->_channels)
        channel->game = Signal<void(const void*)>();
    this->_queue.clear();
    const std::lock_guard lock(this->_inboxMutex);
    this->_inbox.clear();
}

}
