# C++23 feature probe (ADR 0017 · ENG-03): compiles cmake/FeatureProbe.cpp once per allowed feature
# and tells the engine which fallbacks to use. Results are printed at configure time.
#
#   std::expected            missing → ENGINE_USE_TL_EXPECTED (Result uses tl::expected from vcpkg)
#   std::print               missing → ENGINE_NO_STD_PRINT (std::format + fputs)
#   move_only_function, deducing this, to_underlying, unreachable: required, configure stops if missing

include_guard(GLOBAL)

function(_engine_probe feature out)
    try_compile(result
        SOURCES ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/FeatureProbe.cpp
        COMPILE_DEFINITIONS -DPROBE_${feature}
        CMAKE_FLAGS -DCMAKE_CXX_SCAN_FOR_MODULES=OFF
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        OUTPUT_VARIABLE log)
    set(${out} ${result} PARENT_SCOPE)
    if(NOT result)
        file(WRITE ${CMAKE_BINARY_DIR}/feature-probe-${feature}.log "${log}")
    endif()
endfunction()

# Adds the probe's results to <target> as PUBLIC definitions, so every module sees the same choice.
function(engine_probe_cxx23 target)
    set(summary "")
    foreach(feature IN ITEMS EXPECTED PRINT MOVE_ONLY_FUNCTION DEDUCING_THIS TO_UNDERLYING UNREACHABLE)
        _engine_probe(${feature} ok)
        set(ENGINE_HAS_${feature} ${ok} CACHE INTERNAL "C++23 probe: ${feature}")
        if(ok)
            string(APPEND summary " ${feature}=yes")
        else()
            string(APPEND summary " ${feature}=no")
        endif()
    endforeach()
    message(STATUS "C++23 probe (${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}):${summary}")

    foreach(required IN ITEMS MOVE_ONLY_FUNCTION DEDUCING_THIS TO_UNDERLYING UNREACHABLE)
        if(NOT ENGINE_HAS_${required})
            message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} lacks ${required}, required "
                                "by ADR 0017. See ${CMAKE_BINARY_DIR}/feature-probe-${required}.log and guide 01 "
                                "for the supported compilers (GCC 14+, Clang 18+, MSVC 17.10+).")
        endif()
    endforeach()

    if(NOT ENGINE_HAS_EXPECTED)
        message(STATUS "C++23 probe: std::expected missing, Result uses tl::expected")
        find_package(tl-expected CONFIG REQUIRED)
        target_link_libraries(${target} PUBLIC tl::expected)
        target_compile_definitions(${target} PUBLIC ENGINE_USE_TL_EXPECTED)
    endif()
    if(NOT ENGINE_HAS_PRINT)
        message(STATUS "C++23 probe: std::print missing, the engine prints through std::format")
        target_compile_definitions(${target} PUBLIC ENGINE_NO_STD_PRINT)
    endif()
endfunction()
