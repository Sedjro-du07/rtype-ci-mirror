/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SparseArray
*/

#pragma once

#include "SparseArray.hpp"

#include <utility>

namespace engine::bench {

template <class T>
template <class... Args>
T& SparseArray<T>::emplace(Entity e, Args&&... args) {
    if (e.index >= this->_slots.size()) this->_slots.resize(e.index + 1);
    return this->_slots[e.index].emplace(std::forward<Args>(args)...);
}

template <class T>
void SparseArray<T>::remove(Entity e) {
    if (e.index < this->_slots.size()) this->_slots[e.index].reset();
}

template <class T>
bool SparseArray<T>::has(Entity e) const {
    return e.index < this->_slots.size() && this->_slots[e.index].has_value();
}

template <class T>
T& SparseArray<T>::get(Entity e) {
    return *this->_slots[e.index];
}

template <class T>
std::span<std::optional<T>> SparseArray<T>::slots() {
    return this->_slots;
}

}
