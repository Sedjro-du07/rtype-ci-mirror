##
## EPITECH PROJECT, 2026
## G-ING-500-COT-5-1-rtype-2
## File description:
## CMakeLists
##

# Engine unit tests: Catch2 v3, registered with ctest (ADR 0045).
# ENG-53 owns this file and completes it; earlier tasks add their tests with engine_add_test.

find_package(Catch2 3 CONFIG REQUIRED)
include(Catch)

# engine_add_test(<name> SOURCES ...): one test executable linked to the engine and Catch2's main.
function(engine_add_test name)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "SOURCES")
    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE rtype_engine::engine Catch2::Catch2WithMain)
    target_include_directories(${name} PRIVATE ${PROJECT_SOURCE_DIR}/src)   # private headers too
    engine_configure_target(${name})
    # Next to engine.dll: Windows looks for DLLs in the executable's folder.
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        BUILD_RPATH "$ORIGIN")
    catch_discover_tests(${name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin DISCOVERY_MODE PRE_TEST)
endfunction()

engine_add_test(engine_core_tests SOURCES
    core/CoreTypesTest.cpp
    core/SignalTest.cpp
    core/TimeTest.cpp)

engine_add_test(engine_os_tests SOURCES
    os/OsTest.cpp)

engine_add_test(engine_ecs_tests SOURCES
    ecs/EntityTest.cpp
    ecs/SparseSetTest.cpp)
