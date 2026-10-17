##
## EPITECH PROJECT, 2026
## G-ING-500-COT-5-1-rtype-2
## File description:
## EngineOptions
##

# Build options and per-target settings shared by the engine, its plugins and games.
# ENG-01 · ADR 0009 (linking mode), 0017 (C++23), 0044 (portability), 0045 (tooling)

include_guard(GLOBAL)

option(ENGINE_DYNAMIC_MODULES    "Plugins and games as runtime-loaded libraries"  ON)
option(ENGINE_WARNINGS_AS_ERRORS "Treat warnings as errors"                       OFF)
option(ENGINE_SANITIZERS         "Build with address and undefined sanitizers"    OFF)
option(ENGINE_BUILD_TESTS        "Build engine/tests"                             ON)
option(ENGINE_BUILD_BENCHMARKS   "Build engine/benchmarks"                        OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)       # C++20 modules are not on the allow-list (ADR 0017)

# Executables, shared libraries and DLLs land in one folder, so Windows finds every DLL next to the
# executable (guide 24). Plugins and games go to bin/plugins and bin/games (ENG-02).
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

# Warnings: -Wall -Wextra -Wpedantic on GCC and Clang, /W4 on MSVC; errors when requested.
function(engine_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 $<$<BOOL:${ENGINE_WARNINGS_AS_ERRORS}>:/WX>)
    else()
        # -Wno-missing-field-initializers: aggregates with defaults are filled partially on purpose
        # (Error{code, message}, createRoom({.name = …, .level = …})).
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wno-missing-field-initializers
                               $<$<BOOL:${ENGINE_WARNINGS_AS_ERRORS}>:-Werror>)
    endif()
endfunction()

# MSVC in conformance mode, sources in UTF-8 (ADR 0044). No effect on other compilers.
# C4251 / C4275 warn about standard-library members and bases of exported classes; harmless because
# every module uses the same compiler and runtime (/MD, ADR 0002).
function(engine_set_msvc_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /permissive- /Zc:__cplusplus /Zc:preprocessor /EHsc /utf-8
                               /wd4251 /wd4275)
    endif()
endfunction()

function(engine_set_sanitizers target)
    if(ENGINE_SANITIZERS AND NOT MSVC)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

# Everything a library built against the engine needs: C++23, hidden visibility (so a missing
# export fails on Linux as on Windows), warnings, MSVC options, sanitizers.
function(engine_configure_target target)
    target_compile_features(${target} PUBLIC cxx_std_23)
    set_target_properties(${target} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)
    engine_set_warnings(${target})
    engine_set_msvc_options(${target})
    engine_set_sanitizers(${target})
endfunction()
