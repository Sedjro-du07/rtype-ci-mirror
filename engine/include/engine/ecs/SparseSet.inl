/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SparseSet
*/

#pragma once

#include "engine/ecs/SparseSet.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace engine {

template <class T>
SparseSet<T>::SparseSet(ComponentId id) : _id(id) {}

template <class T>
uint32_t SparseSet<T>::position(Entity e) const {
    const size_t page = e.index / PageSize;
    if (page >= this->_sparse.size() || !this->_sparse[page]) return Absent;
    return (*this->_sparse[page])[e.index % PageSize];
}

template <class T>
uint32_t& SparseSet<T>::slot(Entity e) {
    const size_t page = e.index / PageSize;
    if (page >= this->_sparse.size()) this->_sparse.resize(page + 1);
    if (!this->_sparse[page]) {
        this->_sparse[page] = std::make_unique<Page>();
        this->_sparse[page]->fill(Absent);
    }
    return (*this->_sparse[page])[e.index % PageSize];
}

template <class T>
template <class... Args>
T& SparseSet<T>::emplace(Entity e, Args&&... args) {
    assert(!e.isNull() && !this->has(e));
    T& component = this->_data.emplace_back(std::forward<Args>(args)...);
    this->_dense.push_back(e);
    this->slot(e) = static_cast<uint32_t>(this->_dense.size() - 1);
    return component;
}

template <class T>
T& SparseSet<T>::get(Entity e) {
    assert(this->has(e));
    return this->_data[this->position(e)];
}

template <class T>
const T& SparseSet<T>::get(Entity e) const {
    assert(this->has(e));
    return this->_data[this->position(e)];
}

template <class T>
T* SparseSet<T>::tryGet(Entity e) {
    const uint32_t pos = this->position(e);
    return pos != Absent && this->_dense[pos] == e ? &this->_data[pos] : nullptr;
}

template <class T>
T& SparseSet<T>::replace(Entity e, T value) {
    T& component = this->get(e);
    component = std::move(value);
    return component;
}

template <class T>
std::span<T> SparseSet<T>::components() {
    return this->_data;
}

template <class T>
ComponentId SparseSet<T>::id() const {
    return this->_id;
}

template <class T>
bool SparseSet<T>::has(Entity e) const {
    const uint32_t pos = this->position(e);
    return pos != Absent && this->_dense[pos] == e;
}

template <class T>
void SparseSet<T>::remove(Entity e) {
    if (!this->has(e)) return;
    const uint32_t pos = this->position(e);
    const uint32_t last = static_cast<uint32_t>(this->_dense.size() - 1);
    if (pos != last) {
        const Entity moved = this->_dense[last];
        this->_dense[pos] = moved;
        this->_data[pos] = std::move(this->_data[last]);
        this->slot(moved) = pos;
    }
    this->slot(e) = Absent;
    this->_dense.pop_back();
    this->_data.pop_back();
}

template <class T>
size_t SparseSet<T>::size() const {
    return this->_dense.size();
}

template <class T>
void SparseSet<T>::clear() {
    this->_sparse.clear();
    this->_dense.clear();
    this->_data.clear();
}

template <class T>
std::span<const Entity> SparseSet<T>::entities() const {
    return this->_dense;
}

template <class T>
void* SparseSet<T>::getRaw(Entity e) {
    return this->tryGet(e);
}

template <class T>
const void* SparseSet<T>::getRaw(Entity e) const {
    const uint32_t pos = this->position(e);
    return pos != Absent && this->_dense[pos] == e ? &this->_data[pos] : nullptr;
}

template <class T>
void* SparseSet<T>::emplaceRaw(Entity e, const void* src) {
    if constexpr (std::is_copy_constructible_v<T>) {
        return &this->emplace(e, *static_cast<const T*>(src));
    } else {
        assert(false);
        return nullptr;
    }
}

}
