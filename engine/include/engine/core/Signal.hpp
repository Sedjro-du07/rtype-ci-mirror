/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Signal
*/

#pragma once

#include "engine/Export.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

namespace engine {

struct SignalState;
class SignalBase;

class ENGINE_API Subscription {
private:
    friend class SignalBase;

    std::weak_ptr<SignalState> state_;
    uint64_t id_ = 0;

    Subscription(std::weak_ptr<SignalState> state, uint64_t id) noexcept;

public:
    Subscription() = default;
    Subscription(Subscription&& o) noexcept;
    Subscription& operator=(Subscription&& o) noexcept;
    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;
    ~Subscription();

    void reset() noexcept;
    bool active() const noexcept;
};

struct ENGINE_API SignalSlot {
    uint64_t id = 0;
    bool connected = true;

    virtual ~SignalSlot();
};

class ENGINE_API SignalEmission {
private:
    std::shared_ptr<SignalState> state_;
    size_t count_ = 0;

public:
    explicit SignalEmission(std::shared_ptr<SignalState> state);
    SignalEmission(const SignalEmission&) = delete;
    SignalEmission& operator=(const SignalEmission&) = delete;
    ~SignalEmission();

    size_t count() const noexcept;
    SignalSlot* slot(size_t index) const noexcept;
};

class ENGINE_API SignalBase {
private:
    std::shared_ptr<SignalState> state_;

protected:
    Subscription attach(std::unique_ptr<SignalSlot> slot);
    SignalEmission beginEmit() const;

public:
    size_t size() const;
};

template <class Sig>
class Signal;

template <class... Args>
class Signal<void(Args...)> : private SignalBase {
private:
    using Listener = std::move_only_function<void(Args...)>;

    struct Slot : SignalSlot {
        Listener fn;

        explicit Slot(Listener f);
    };

public:
    Signal() = default;
    Signal(Signal&&) noexcept = default;
    Signal& operator=(Signal&&) noexcept = default;
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    [[nodiscard]] Subscription connect(Listener fn);
    void emit(Args... args);
    using SignalBase::size;
};

}

#include "engine/core/Signal.inl"
