/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Registry
*/

#pragma once

#include "engine/ecs/Registry.hpp"

#include <cassert>
#include <tuple>
#include <utility>
#include <vector>

namespace engine {

template <class T>
ComponentId Registry::componentId() const {
    static_assert(HasTypeName<T>, "Registry: T has no type macro; place ENGINE_COMPONENT after it");
    static_assert(TypeName<T>::category == TypeCategory::Component,
                  "Registry: T is not a component; declare it with ENGINE_COMPONENT");
    const TypeInfo* info = this->_types->findByKey(typeKey<T>());
    if (!info) detail::unregisteredComponent(TypeName<T>::value);
    return info->id;
}

template <class T>
SparseSet<T>& Registry::assure() {
    return static_cast<SparseSet<T>&>(this->assure(this->componentId<T>()));
}

template <class T>
SparseSet<T>* Registry::existing() const {
    const IStorage* found = this->storage(this->componentId<T>());
    return const_cast<SparseSet<T>*>(static_cast<const SparseSet<T>*>(found));
}

template <class T, class... Args>
T& Registry::emplace(Entity e, Args&&... args) {
    assert(this->alive(e));
    SparseSet<T>& set = this->assure<T>();
    set.emplace(e, std::forward<Args>(args)...);
    this->fire(&ComponentHooks::onCreate, set.id(), e);
    return this->get<T>(e);
}

template <class T>
T& Registry::get(Entity e) {
    SparseSet<T>* set = this->existing<T>();
    assert(set && set->has(e));
    return set->get(e);
}

template <class T>
const T& Registry::get(Entity e) const {
    const SparseSet<T>* set = this->existing<T>();
    assert(set && set->has(e));
    return set->get(e);
}

template <class T>
T* Registry::tryGet(Entity e) {
    SparseSet<T>* set = this->existing<T>();
    return set ? set->tryGet(e) : nullptr;
}

template <class T>
bool Registry::has(Entity e) const {
    const SparseSet<T>* set = this->existing<T>();
    return set && set->has(e);
}

template <class T>
T& Registry::replace(Entity e, T value) {
    SparseSet<T>& set = this->assure<T>();
    set.replace(e, std::move(value));
    this->fire(&ComponentHooks::onReplace, set.id(), e);
    return this->get<T>(e);
}

template <class T>
void Registry::remove(Entity e) {
    this->removeRaw(e, this->componentId<T>());
}

template <class... Ts>
View<Ts...> Registry::view() {
    return View<Ts...>(std::tuple<SparseSet<Ts>*...>(this->existing<Ts>()...), {});
}

template <class... Ts, class... Xs>
View<Ts...> Registry::view(Exclude<Xs...>) {
    std::vector<const IStorage*> excluded{this->storage(this->componentId<Xs>())...};
    return View<Ts...>(std::tuple<SparseSet<Ts>*...>(this->existing<Ts>()...), std::move(excluded));
}

template <class T>
T& Registry::singleton() {
    if (!this->alive(this->_singletons)) this->_singletons = this->create();
    if (T* found = this->tryGet<T>(this->_singletons)) return *found;
    return this->emplace<T>(this->_singletons);
}

template <class T>
T* Registry::trySingleton() {
    return this->alive(this->_singletons) ? this->tryGet<T>(this->_singletons) : nullptr;
}

template <class T>
ComponentHooks& Registry::hooks() {
    return this->hooks(this->componentId<T>());
}

}
