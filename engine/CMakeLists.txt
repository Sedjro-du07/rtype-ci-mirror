##
## EPITECH PROJECT, 2026
## G-ING-500-COT-5-1-rtype-2
## File description:
## CMakeLists
##

# The engine library. Builds alone from engine/ or as part of the root project.
# ENG-01 · ADR 0009, 0017, 0044, 0045 · docs/engine/reference/02-modules.md

cmake_minimum_required(VERSION 3.25)
project(rtype_engine VERSION 0.1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(ENGINE_BUILDING_ENGINE ON)  # plugins declared below join the engine's install export set (ENG-62)
include(EngineOptions)
include(EngineModules)          # engine_add_plugin, engine_add_game_module, engine_add_launcher (ENG-02)
include(EngineFeatures)         # C++23 probe and fallbacks (ENG-03)

# Dynamic mode: one shared engine used by the executable, every plugin and the game (ADR 0044).
# Static mode: linked into one executable (ADR 0009).
if(ENGINE_DYNAMIC_MODULES)
    set(ENGINE_LIBRARY_KIND SHARED)
else()
    set(ENGINE_LIBRARY_KIND STATIC)
endif()

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS include/*.hpp include/*.inl src/*.hpp)

add_library(engine ${ENGINE_LIBRARY_KIND} ${ENGINE_SOURCES} ${ENGINE_HEADERS})
add_library(rtype_engine::engine ALIAS engine)
engine_configure_target(engine)
engine_probe_cxx23(engine)

# ENGINE_API: dllexport while building the engine, dllimport for its users, default visibility on
# Linux, empty in static mode. Written to the build tree as engine/Export.hpp (ADR 0044).
include(GenerateExportHeader)
generate_export_header(engine
    BASE_NAME ENGINE
    EXPORT_MACRO_NAME ENGINE_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/engine/Export.hpp)
if(NOT ENGINE_DYNAMIC_MODULES)
    target_compile_definitions(engine PUBLIC ENGINE_STATIC_DEFINE ENGINE_STATIC_MODULES)
endif()

if(WIN32)
    target_link_libraries(engine PRIVATE bcrypt ws2_32)
endif()

target_include_directories(engine
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Third-party libraries are linked by the task that first uses them; vcpkg.json lists them all.
# Never let a third-party type into include/engine (reference README).

find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(engine PRIVATE yaml-cpp::yaml-cpp)

# Plugins: one sub-directory each, built by engine_add_plugin (ENG-02 and the plugin tasks).
file(GLOB ENGINE_PLUGIN_DIRS LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/plugins/*)
foreach(dir IN LISTS ENGINE_PLUGIN_DIRS)
    if(EXISTS ${dir}/CMakeLists.txt)
        add_subdirectory(${dir})
    endif()
endforeach()

if(ENGINE_BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
    enable_testing()
    add_subdirectory(tests)
endif()
if(ENGINE_BUILD_BENCHMARKS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/CMakeLists.txt)
    add_subdirectory(benchmarks)
endif()

# find_package(rtype_engine) support and CPack helpers (ENG-62).
include(EngineInstall)
engine_install_engine()
