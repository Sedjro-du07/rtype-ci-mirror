##
## EPITECH PROJECT, 2026
## G-ING-500-COT-5-1-rtype-2
## File description:
## CMakeLists
##

# R-Type: the engine and, later, the game. Configure with a preset: cmake --preset linux-gcc-debug
# ENG-01 · ADR 0045

cmake_minimum_required(VERSION 3.25)
project(rtype VERSION 0.1.0 LANGUAGES CXX)

enable_testing()
add_subdirectory(engine)
# add_subdirectory(rtype)      # the game (Game part)

# cpack --preset <preset>: an archive of what `cmake --install` puts in place (ENG-62, guide 24).
engine_enable_packaging(NAME r-type VENDOR "R-Type team")
