/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SignalTest
*/

#include "engine/core/Signal.hpp"

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using namespace engine;

TEST_CASE("Signal calls listeners in connection order with the arguments", "[core][signal]") {
    Signal<void(int, const std::string&)> sig;
    std::vector<std::string> calls;
    const auto a = sig.connect([&](int n, const std::string& s) { calls.push_back("a" + std::to_string(n) + s); });
    const auto b = sig.connect([&](int n, const std::string& s) { calls.push_back("b" + std::to_string(n) + s); });
    CHECK(sig.size() == 2);

    sig.emit(1, "x");
    CHECK(calls == std::vector<std::string>{"a1x", "b1x"});
}

TEST_CASE("Signal without listeners and empty functions", "[core][signal]") {
    Signal<void()> sig;
    CHECK(sig.size() == 0);
    sig.emit();

    const auto sub = sig.connect(nullptr);
    CHECK_FALSE(sub.active());
    CHECK(sig.size() == 0);
}

TEST_CASE("Destroying or resetting the Subscription stops calls", "[core][signal]") {
    Signal<void()> sig;
    int a = 0, b = 0;
    auto subA = sig.connect([&] { ++a; });
    {
        const auto subB = sig.connect([&] { ++b; });
        sig.emit();
    }
    sig.emit();
    CHECK(a == 2);
    CHECK(b == 1);

    CHECK(subA.active());
    subA.reset();
    CHECK_FALSE(subA.active());
    CHECK(sig.size() == 0);
    sig.emit();
    CHECK(a == 2);
    subA.reset();
}

TEST_CASE("Subscription moves", "[core][signal]") {
    Signal<void()> sig;
    int a = 0, b = 0;
    auto subA = sig.connect([&] { ++a; });
    auto subB = sig.connect([&] { ++b; });

    Subscription moved = std::move(subA);
    CHECK(moved.active());
    CHECK_FALSE(subA.active());
    CHECK(sig.size() == 2);

    moved = std::move(subB);
    CHECK(sig.size() == 1);
    sig.emit();
    CHECK(a == 0);
    CHECK(b == 1);

    std::vector<Subscription> kept;
    kept.push_back(std::move(moved));
    kept.clear();
    CHECK(sig.size() == 0);
}

TEST_CASE("A listener can disconnect itself or a later listener during emit", "[core][signal]") {
    Signal<void()> sig;
    int self = 0, later = 0, last = 0;
    Subscription selfSub, laterSub;
    selfSub = sig.connect([&] {
        ++self;
        selfSub.reset();
        laterSub.reset();
    });
    laterSub = sig.connect([&] { ++later; });
    const auto lastSub = sig.connect([&] { ++last; });

    sig.emit();
    CHECK(self == 1);
    CHECK(later == 0);
    CHECK(last == 1);
    CHECK(sig.size() == 1);

    sig.emit();
    CHECK(self == 1);
    CHECK(last == 2);
}

TEST_CASE("A listener connected during emit is called from the next emit", "[core][signal]") {
    Signal<void()> sig;
    int added = 0;
    std::vector<Subscription> subs;
    subs.push_back(sig.connect([&] {
        if (subs.size() < 50) subs.push_back(sig.connect([&] { ++added; }));
    }));

    sig.emit();
    CHECK(added == 0);
    CHECK(sig.size() == 2);
    sig.emit();
    CHECK(added == 1);
}

TEST_CASE("emit inside emit", "[core][signal]") {
    Signal<void(int)> sig;
    std::vector<int> calls;
    Subscription removed;
    const auto outer = sig.connect([&](int depth) {
        calls.push_back(depth);
        if (depth == 0) {
            sig.emit(1);
            removed.reset();
        }
    });
    removed = sig.connect([&](int depth) { calls.push_back(10 + depth); });

    sig.emit(0);
    CHECK(calls == std::vector<int>{0, 1, 11});
    CHECK(sig.size() == 1);
}

TEST_CASE("Signal destroyed before its Subscriptions", "[core][signal]") {
    Subscription sub;
    {
        Signal<void()> sig;
        sub = sig.connect([] {});
        CHECK(sub.active());
    }
    CHECK_FALSE(sub.active());
    sub.reset();
}

TEST_CASE("A listener can destroy the Signal it is called from", "[core][signal]") {
    auto sig = std::make_unique<Signal<void()>>();
    int after = 0;
    const auto a = sig->connect([&] { sig.reset(); });
    const auto b = sig->connect([&] { ++after; });
    sig->emit();
    CHECK(sig == nullptr);
    CHECK(after == 1);
    CHECK_FALSE(a.active());
}

TEST_CASE("Move-only listeners", "[core][signal]") {
    Signal<void(int&)> sig;
    const auto sub = sig.connect([p = std::make_unique<int>(41)](int& out) { out = *p + 1; });
    int out = 0;
    sig.emit(out);
    CHECK(out == 42);
}

TEST_CASE("A moved Signal keeps its listeners", "[core][signal]") {
    Signal<void()> a;
    int calls = 0;
    const auto sub = a.connect([&] { ++calls; });
    Signal<void()> b = std::move(a);
    b.emit();
    CHECK(calls == 1);
    CHECK(sub.active());
}
