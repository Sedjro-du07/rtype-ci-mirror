/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** View
*/

#pragma once

#include "engine/ecs/View.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

namespace engine {

template <class... Ts>
View<Ts...>::View(std::tuple<SparseSet<Ts>*...> sets, std::vector<const IStorage*> excluded)
    : _sets(sets), _excluded(std::move(excluded)) {
    std::erase(this->_excluded, nullptr);
    const bool complete = std::apply([](auto*... set) { return ((set != nullptr) && ...); }, this->_sets);
    if (!complete) return;
    size_t smallest = SIZE_MAX;
    const auto consider = [this, &smallest](const IStorage* storage) {
        if (storage->size() >= smallest) return;
        smallest = storage->size();
        this->_driver = storage->entities();
    };
    std::apply([&consider](auto*... set) { (consider(set), ...); }, this->_sets);
}

template <class... Ts>
bool View<Ts...>::matches(Entity e) const {
    const bool hasAll = std::apply([e](auto*... set) { return (set->has(e) && ...); }, this->_sets);
    if (!hasAll) return false;
    return std::ranges::none_of(this->_excluded, [e](const IStorage* storage) { return storage->has(e); });
}

template <class... Ts>
View<Ts...>::iterator::iterator(const View* view, size_t index) : _view(view), _index(index) {
    this->skip();
}

template <class... Ts>
void View<Ts...>::iterator::skip() {
    while (this->_index < this->_view->_driver.size() && !this->_view->matches(this->_view->_driver[this->_index]))
        ++this->_index;
}

template <class... Ts>
typename View<Ts...>::iterator::value_type View<Ts...>::iterator::operator*() const {
    const Entity e = this->_view->_driver[this->_index];
    return std::apply([e](auto*... set) { return value_type{e, set->get(e)...}; }, this->_view->_sets);
}

template <class... Ts>
typename View<Ts...>::iterator& View<Ts...>::iterator::operator++() {
    ++this->_index;
    this->skip();
    return *this;
}

template <class... Ts>
typename View<Ts...>::iterator View<Ts...>::iterator::operator++(int) {
    iterator copy = *this;
    ++*this;
    return copy;
}

template <class... Ts>
bool View<Ts...>::iterator::operator==(const iterator& other) const {
    return this->_index == other._index;
}

template <class... Ts>
typename View<Ts...>::iterator View<Ts...>::begin() {
    return iterator(this, 0);
}

template <class... Ts>
typename View<Ts...>::iterator View<Ts...>::end() {
    return iterator(this, this->_driver.size());
}

template <class... Ts>
size_t View<Ts...>::sizeHint() const {
    return this->_driver.size();
}

template <class... Ts>
template <class Fn>
void View<Ts...>::each(Fn&& fn) {
    for (auto components : *this)
        std::apply(fn, components);
}

template <class... Ts>
Entity View<Ts...>::front() {
    const iterator first = this->begin();
    return first == this->end() ? Entity::null() : std::get<0>(*first);
}

}
