##
## EPITECH PROJECT, 2026
## G-ING-500-COT-5-1-rtype-2
## File description:
## CMakeLists
##

add_executable(engine_bench_ecs_storage
    ArchetypeStore.cpp
    EcsStorageBench.cpp)
target_link_libraries(engine_bench_ecs_storage PRIVATE rtype_engine::engine benchmark::benchmark_main)
engine_configure_target(engine_bench_ecs_storage)
set_target_properties(engine_bench_ecs_storage PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    BUILD_RPATH "$ORIGIN")
