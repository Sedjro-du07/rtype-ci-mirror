/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EventBus
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Signal.hpp"
#include "engine/reflection/TypeKey.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace engine {

class ENGINE_API EventBus {
private:
    using Delivery = std::move_only_function<void()>;
    using Listener = std::move_only_function<void(const void*)>;

    struct Channel {
        Signal<void(const void*)> engine;
        Signal<void(const void*)> game;
    };

    std::unordered_map<uint64_t, std::unique_ptr<Channel>> _channels;
    std::vector<Delivery> _queue;
    std::vector<Delivery> _inbox;
    std::mutex _inboxMutex;
    bool _gamePhase = false;

    template <class E>
    static constexpr uint64_t keyOf();

    Subscription subscribeRaw(uint64_t key, Listener listener);
    void deliver(uint64_t key, const void* event);
    void post(Delivery delivery);
    void postFromAnyThread(Delivery delivery);

public:
    EventBus() = default;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    template <class E>
    [[nodiscard]] Subscription subscribe(std::move_only_function<void(const E&)> fn);
    template <class E>
    void publish(E event);
    template <class E>
    void emit(const E& event);
    template <class E>
    void publishFromAnyThread(E event);

    void dispatch();
    void beginGameSubscriptions();
    void clearGameSubscriptions();
};

}

#include "engine/events/EventBus.inl"
