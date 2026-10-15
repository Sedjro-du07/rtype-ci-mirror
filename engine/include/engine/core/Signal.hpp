/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Signal
*/

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace engine {

template <class Sig>
class Signal;

class Subscription {
private:
    template <class Sig>
    friend class Signal;

    using Disconnect = void (*)(void* state, uint64_t id) noexcept;

    std::weak_ptr<void> state_;
    Disconnect disconnect_ = nullptr;
    uint64_t id_ = 0;

    Subscription(std::weak_ptr<void> state, Disconnect disconnect, uint64_t id) noexcept
        : state_(std::move(state)), disconnect_(disconnect), id_(id) {}

public:
    Subscription() = default;
    Subscription(Subscription&& o) noexcept
        : state_(std::move(o.state_)), disconnect_(std::exchange(o.disconnect_, nullptr)),
          id_(std::exchange(o.id_, 0)) {}
    Subscription& operator=(Subscription&& o) noexcept {
        if (this != &o) {
            reset();
            state_ = std::move(o.state_);
            disconnect_ = std::exchange(o.disconnect_, nullptr);
            id_ = std::exchange(o.id_, 0);
        }
        return *this;
    }
    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;
    ~Subscription() { reset(); }

    void reset() noexcept {
        if (const auto state = state_.lock()) disconnect_(state.get(), id_);
        state_.reset();
        disconnect_ = nullptr;
        id_ = 0;
    }

    bool active() const noexcept { return disconnect_ != nullptr && !state_.expired(); }
};

template <class... Args>
class Signal<void(Args...)> {
private:
    using Listener = std::move_only_function<void(Args...)>;

    struct Slot {
        Slot(uint64_t i, Listener f) : id(i), fn(std::move(f)) {}
        uint64_t id;
        Listener fn;
        bool connected = true;
    };

    struct State {
        std::vector<std::unique_ptr<Slot>> slots;
        uint64_t nextId = 1;
        size_t alive = 0;
        int emitting = 0;
    };

    struct EmitScope {
        explicit EmitScope(State& s) : state(s) { ++state.emitting; }
        ~EmitScope() {
            if (--state.emitting == 0 && state.alive != state.slots.size())
                std::erase_if(state.slots, [](const auto& slot) { return !slot->connected; });
        }
        EmitScope(const EmitScope&) = delete;
        EmitScope& operator=(const EmitScope&) = delete;
        State& state;
    };

    std::shared_ptr<State> state_;

    static void disconnect(void* p, uint64_t id) noexcept {
        State& state = *static_cast<State*>(p);
        const auto it = std::ranges::find_if(state.slots, [id](const auto& slot) { return slot->id == id; });
        if (it == state.slots.end() || !(*it)->connected) return;
        (*it)->connected = false;
        --state.alive;
        if (state.emitting == 0) state.slots.erase(it);
    }

public:
    Signal() = default;
    Signal(Signal&&) noexcept = default;
    Signal& operator=(Signal&&) noexcept = default;
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    [[nodiscard]] Subscription connect(Listener fn) {
        if (!fn) return {};
        if (!state_) state_ = std::make_shared<State>();
        const uint64_t id = state_->nextId++;
        state_->slots.push_back(std::make_unique<Slot>(id, std::move(fn)));
        ++state_->alive;
        return Subscription(state_, &Signal::disconnect, id);
    }

    void emit(Args... args) {
        if (!state_) return;
        const auto state = state_;
        const EmitScope scope{*state};
        const size_t count = state->slots.size();
        for (size_t i = 0; i < count; ++i) {
            Slot& slot = *state->slots[i];
            if (slot.connected) slot.fn(args...);
        }
    }

    size_t size() const { return state_ ? state_->alive : 0; }
};

}
