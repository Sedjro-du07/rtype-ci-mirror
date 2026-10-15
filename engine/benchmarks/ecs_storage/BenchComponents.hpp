/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** BenchComponents
*/

#pragma once

namespace engine::bench {

struct Position {
    float x = 0, y = 0;
};

struct Velocity {
    float x = 1, y = 1;
};

struct Health {
    int value = 100;
};

}
