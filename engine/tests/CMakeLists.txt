##
## EPITECH PROJECT, 2026
## G-ING-500-COT-5-1-rtype-2
## File description:
## CMakeLists
##

# Engine unit tests: Catch2 v3, registered with ctest (ADR 0045).
# ENG-53 owns the full setup; each SRV/ENG task appends its own engine_add_test call.

find_package(Catch2 3 CONFIG REQUIRED)
include(Catch)

# engine_add_test(<name> SOURCES ...): one test executable linked to the engine and Catch2's main.
function(engine_add_test name)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "SOURCES")
    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE rtype_engine::engine Catch2::Catch2WithMain)
    target_include_directories(${name} PRIVATE ${PROJECT_SOURCE_DIR}/src)
    engine_configure_target(${name})
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        BUILD_RPATH "$ORIGIN")
    catch_discover_tests(${name} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin DISCOVERY_MODE PRE_TEST)
endfunction()

# ENG-24 / SRV-04 — ByteStream and Codec tests
engine_add_test(engine_reflection_tests SOURCES
    reflection/ByteStreamTest.cpp
    reflection/codec/CodecTest.cpp)
