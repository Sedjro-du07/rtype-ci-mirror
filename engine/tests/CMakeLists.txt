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
    ecs/CommandsTest.cpp
    ecs/EntityTest.cpp
    ecs/HooksTest.cpp
    ecs/RegistryTest.cpp
    ecs/SparseSetTest.cpp
    ecs/ViewTest.cpp)

engine_add_test(engine_reflection_tests SOURCES
    reflection/TypeRegistryTest.cpp)

foreach(case IN ITEMS 1 2 3)
    add_executable(engine_compile_fail_${case} EXCLUDE_FROM_ALL reflection/CompileFail.cpp)
    target_link_libraries(engine_compile_fail_${case} PRIVATE rtype_engine::engine)
    target_compile_definitions(engine_compile_fail_${case} PRIVATE ENGINE_COMPILE_FAIL_CASE=${case})
    engine_configure_target(engine_compile_fail_${case})
    add_test(NAME "compile fails: reflection case ${case}"
             COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target engine_compile_fail_${case})
endforeach()
set_tests_properties("compile fails: reflection case 1" PROPERTIES
    PASS_REGULAR_EXPRESSION "has no type macro" RUN_SERIAL TRUE)
set_tests_properties("compile fails: reflection case 2" PROPERTIES
    PASS_REGULAR_EXPRESSION "member.*flags|flags.*member" RUN_SERIAL TRUE)
set_tests_properties("compile fails: reflection case 3" PROPERTIES
    PASS_REGULAR_EXPRESSION "unsupported field type" RUN_SERIAL TRUE)

add_executable(engine_unregistered_component ecs/UnregisteredComponent.cpp)
target_link_libraries(engine_unregistered_component PRIVATE rtype_engine::engine)
engine_configure_target(engine_unregistered_component)
set_target_properties(engine_unregistered_component PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    BUILD_RPATH "$ORIGIN")
add_test(NAME "unregistered component is reported" COMMAND engine_unregistered_component
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_tests_properties("unregistered component is reported" PROPERTIES
    PASS_REGULAR_EXPRESSION "component 'Unregistered' is not registered")

function(engine_add_test_module target)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "SOURCES")
    if(ENGINE_DYNAMIC_MODULES)
        add_library(${target} SHARED ${ARG_SOURCES})
        target_compile_definitions(${target} PRIVATE
            "ENGINE_EXPORT=$<IF:$<CXX_COMPILER_ID:MSVC>,__declspec(dllexport),__attribute__((visibility(\"default\")))>")
    else()
        add_library(${target} STATIC ${ARG_SOURCES})
    endif()
    target_link_libraries(${target} PUBLIC rtype_engine::engine)
    target_compile_definitions(${target} PRIVATE ENGINE_MODULE_NAME="${target}")
    engine_configure_target(${target})
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        BUILD_RPATH "$ORIGIN")
endfunction()

engine_add_test_module(game_fixture SOURCES module/FixtureGame.cpp)
engine_add_test_module(game_fixture_throw SOURCES module/ThrowingGame.cpp)
engine_add_test_module(network_fixture SOURCES module/FixturePlugin.cpp)

engine_add_test(engine_module_tests SOURCES
    module/ModuleTest.cpp)
engine_add_test(engine_module_throw_tests SOURCES
    module/ThrowingModuleTest.cpp)
foreach(pair IN ITEMS "engine_module_tests:game_fixture" "engine_module_tests:network_fixture"
                      "engine_module_throw_tests:game_fixture_throw")
    string(REPLACE ":" ";" pair "${pair}")
    list(GET pair 0 test)
    list(GET pair 1 module)
    if(ENGINE_DYNAMIC_MODULES)
        target_link_libraries(${test} PRIVATE ${module})
    else()
        target_link_libraries(${test} PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,${module}>")
    endif()
endforeach()

engine_add_test(engine_render_tests SOURCES
    render/RenderTest.cpp)
