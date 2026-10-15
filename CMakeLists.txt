# R-Type: the engine and, later, the game. Configure with a preset: cmake --preset linux-gcc-debug
# ENG-01 · ADR 0045

cmake_minimum_required(VERSION 3.25)
project(rtype VERSION 0.1.0 LANGUAGES CXX)

enable_testing()
add_subdirectory(engine)
# add_subdirectory(rtype)      # the game (Game part)
