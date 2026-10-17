/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Signal
*/

#include "engine/core/Signal.hpp"

#include <algorithm>
#include <utility>
#include <vector>

namespace engine {

struct SignalState {
    std::vector<std::unique_ptr<SignalSlot>> slots;
    uint64_t nextId = 1;
    size_t alive = 0;
    int emitting = 0;

    void disconnect(uint64_t id) noexcept {
        const auto it = std::ranges::find_if(slots, [id](const auto& slot) { return slot->id == id; });
        if (it == slots.end() || !(*it)->connected) return;
        (*it)->connected = false;
        --alive;
        if (emitting == 0) slots.erase(it);
    }
};

Subscription::Subscription(std::weak_ptr<SignalState> state, uint64_t id) noexcept
    : state_(std::move(state)), id_(id) {}

Subscription::Subscription(Subscription&& o) noexcept : state_(std::move(o.state_)), id_(std::exchange(o.id_, 0)) {}

Subscription& Subscription::operator=(Subscription&& o) noexcept {
    if (this != &o) {
        reset();
        state_ = std::move(o.state_);
        id_ = std::exchange(o.id_, 0);
    }
    return *this;
}

Subscription::~Subscription() {
    reset();
}

void Subscription::reset() noexcept {
    if (const auto state = state_.lock()) state->disconnect(id_);
    state_.reset();
    id_ = 0;
}

bool Subscription::active() const noexcept {
    return id_ != 0 && !state_.expired();
}

SignalSlot::~SignalSlot() = default;

SignalEmission::SignalEmission(std::shared_ptr<SignalState> state) : state_(std::move(state)) {
    if (!state_) return;
    ++state_->emitting;
    count_ = state_->slots.size();
}

SignalEmission::~SignalEmission() {
    if (!state_) return;
    if (--state_->emitting == 0 && state_->alive != state_->slots.size())
        std::erase_if(state_->slots, [](const auto& slot) { return !slot->connected; });
}

size_t SignalEmission::count() const noexcept {
    return count_;
}

SignalSlot* SignalEmission::slot(size_t index) const noexcept {
    SignalSlot* slot = state_->slots[index].get();
    return slot->connected ? slot : nullptr;
}

Subscription SignalBase::attach(std::unique_ptr<SignalSlot> slot) {
    if (!state_) state_ = std::make_shared<SignalState>();
    slot->id = state_->nextId++;
    const uint64_t id = slot->id;
    state_->slots.push_back(std::move(slot));
    ++state_->alive;
    return Subscription(state_, id);
}

SignalEmission SignalBase::beginEmit() const {
    return SignalEmission(state_);
}

size_t SignalBase::size() const {
    return state_ ? state_->alive : 0;
}

}
