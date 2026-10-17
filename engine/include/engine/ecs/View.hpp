/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** View
*/

#pragma once

#include "engine/ecs/Entity.hpp"
#include "engine/ecs/IStorage.hpp"
#include "engine/ecs/SparseSet.hpp"

#include <cstddef>
#include <span>
#include <tuple>
#include <vector>

namespace engine {

template <class... Xs>
struct Exclude {};

template <class... Xs>
inline constexpr Exclude<Xs...> exclude{};

template <class... Ts>
class View {
    static_assert(sizeof...(Ts) > 0, "a view needs at least one component type");

private:
    std::tuple<SparseSet<Ts>*...> _sets;
    std::vector<const IStorage*> _excluded;
    std::span<const Entity> _driver;

    bool matches(Entity e) const;

public:
    class iterator {
    private:
        const View* _view = nullptr;
        size_t _index = 0;

        void skip();

    public:
        using value_type = std::tuple<Entity, Ts&...>;
        using difference_type = std::ptrdiff_t;

        iterator() = default;
        iterator(const View* view, size_t index);

        value_type operator*() const;
        iterator& operator++();
        iterator operator++(int);
        bool operator==(const iterator& other) const;
    };

    View(std::tuple<SparseSet<Ts>*...> sets, std::vector<const IStorage*> excluded);

    iterator begin();
    iterator end();
    size_t sizeHint() const;
    template <class Fn>
    void each(Fn&& fn);
    Entity front();
};

}

#include "engine/ecs/View.inl"
