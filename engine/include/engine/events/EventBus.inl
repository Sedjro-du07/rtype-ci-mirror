/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EventBus
*/

#pragma once

#include "engine/events/EventBus.hpp"

#include <utility>

namespace engine {

template <class E>
constexpr uint64_t EventBus::keyOf() {
    static_assert(HasTypeName<E>, "EventBus: E has no type macro; place ENGINE_EVENT after it");
    static_assert(TypeName<E>::category == TypeCategory::Event, "EventBus: E is not an event; use ENGINE_EVENT");
    return typeKey<E>();
}

template <class E>
Subscription EventBus::subscribe(std::move_only_function<void(const E&)> fn) {
    return this->subscribeRaw(keyOf<E>(),
                              [fn = std::move(fn)](const void* event) mutable { fn(*static_cast<const E*>(event)); });
}

template <class E>
void EventBus::publish(E event) {
    this->post([this, event = std::move(event)]() { this->deliver(keyOf<E>(), &event); });
}

template <class E>
void EventBus::emit(const E& event) {
    this->deliver(keyOf<E>(), &event);
}

template <class E>
void EventBus::publishFromAnyThread(E event) {
    this->postFromAnyThread([this, event = std::move(event)]() { this->deliver(keyOf<E>(), &event); });
}

}
