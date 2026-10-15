/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Signal
*/

#include <utility>

namespace engine {

template <class... Args>
Signal<void(Args...)>::Slot::Slot(Listener f) : fn(std::move(f)) {}

template <class... Args>
Subscription Signal<void(Args...)>::connect(Listener fn) {
    if (!fn) return {};
    return attach(std::make_unique<Slot>(std::move(fn)));
}

template <class... Args>
void Signal<void(Args...)>::emit(Args... args) {
    const SignalEmission emission = beginEmit();
    for (size_t i = 0; i < emission.count(); ++i)
        if (SignalSlot* slot = emission.slot(i)) static_cast<Slot*>(slot)->fn(args...);
}

}
