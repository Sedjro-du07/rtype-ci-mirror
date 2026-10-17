/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Scheduler
*/

#pragma once

#include "engine/scheduler/Scheduler.hpp"

#include <concepts>
#include <string>
#include <type_traits>
#include <utility>

namespace engine {

template <class F>
SystemBuilder Scheduler::add(Stage stage, std::string name, F fn) {
    static_assert(std::is_copy_constructible_v<F>,
                  "a system must be copyable: each room gets its own copy (use addFactory otherwise)");
    static_assert(std::is_invocable_v<F&, Context&>, "a system is called with Context&");
    return this->addFactory(stage, std::move(name), [fn = std::move(fn)]() { return SystemFn(fn); });
}

}
