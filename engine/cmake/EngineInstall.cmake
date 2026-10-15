##
## EPITECH PROJECT, 2026
## G-ING-500-COT-5-1-rtype-2
## File description:
## EngineInstall
##

# Install the engine so another project can use find_package(rtype_engine), and package a game.
# ENG-62 · ADR 0045 · guide 24
#
#   cmake --install build/<preset> --prefix <dir>
#     <dir>/include/engine/…                 public headers + the generated Export.hpp
#     <dir>/lib/ (and bin/ for DLLs)         the engine library
#     <dir>/bin/plugins/                     the engine's plugins (dynamic mode)
#     <dir>/lib/cmake/rtype_engine/          rtype_engineConfig.cmake, targets, CMake functions, launcher
#
# Plugins built with engine_add_plugin join the export set; games and launchers built with
# engine_add_game_module / engine_add_launcher are installed to bin/games and bin.

include_guard(GLOBAL)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(ENGINE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/rtype_engine)

function(engine_install_engine)
    install(TARGETS engine EXPORT rtype_engineTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.hpp")
    install(FILES ${PROJECT_BINARY_DIR}/include/engine/Export.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/engine)

    install(EXPORT rtype_engineTargets NAMESPACE rtype_engine:: DESTINATION ${ENGINE_INSTALL_CMAKEDIR})
    install(FILES
        ${PROJECT_SOURCE_DIR}/cmake/EngineOptions.cmake
        ${PROJECT_SOURCE_DIR}/cmake/EngineModules.cmake
        ${PROJECT_SOURCE_DIR}/cmake/EngineFeatures.cmake
        ${PROJECT_SOURCE_DIR}/cmake/EngineInstall.cmake
        ${PROJECT_SOURCE_DIR}/cmake/FeatureProbe.cpp
        DESTINATION ${ENGINE_INSTALL_CMAKEDIR})
    install(FILES ${PROJECT_SOURCE_DIR}/launcher/main.cpp DESTINATION ${ENGINE_INSTALL_CMAKEDIR}/launcher)

    get_property(plugins GLOBAL PROPERTY ENGINE_PLUGIN_TARGETS)
    set(ENGINE_INSTALLED_PLUGINS "")
    foreach(p IN LISTS plugins)
        list(APPEND ENGINE_INSTALLED_PLUGINS rtype_engine::${p})
    endforeach()
    get_target_property(ENGINE_COMPILE_DEFINITIONS engine INTERFACE_COMPILE_DEFINITIONS)
    if(ENGINE_COMPILE_DEFINITIONS MATCHES "ENGINE_USE_TL_EXPECTED")
        set(ENGINE_NEEDS_TL_EXPECTED ON)
    else()
        set(ENGINE_NEEDS_TL_EXPECTED OFF)
    endif()

    configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/rtype_engineConfig.cmake.in
        ${PROJECT_BINARY_DIR}/rtype_engineConfig.cmake
        INSTALL_DESTINATION ${ENGINE_INSTALL_CMAKEDIR})
    write_basic_package_version_file(
        ${PROJECT_BINARY_DIR}/rtype_engineConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMinorVersion)
    install(FILES
        ${PROJECT_BINARY_DIR}/rtype_engineConfig.cmake
        ${PROJECT_BINARY_DIR}/rtype_engineConfigVersion.cmake
        DESTINATION ${ENGINE_INSTALL_CMAKEDIR})
endfunction()

# Archives of a game (guide 24): call once from the top-level CMakeLists.txt.
#   engine_enable_packaging(NAME r-type VENDOR "R-Type team")
#   cpack --preset linux-gcc-release      → r-type-<version>-Linux.tar.gz  (Windows: .zip)
function(engine_enable_packaging)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "NAME;VENDOR" "")
    set(version "${CMAKE_PROJECT_VERSION}")
    if(NOT version)
        set(version 0.0.0)                                  # top-level project() without VERSION
    endif()
    set(CPACK_PACKAGE_NAME ${ARG_NAME})
    set(CPACK_PACKAGE_VENDOR "${ARG_VENDOR}")
    set(CPACK_PACKAGE_VERSION ${version})
    if(WIN32)
        set(CPACK_GENERATOR ZIP)
    else()
        set(CPACK_GENERATOR TGZ)
    endif()
    set(CPACK_PACKAGE_FILE_NAME "${ARG_NAME}-${version}-${CMAKE_SYSTEM_NAME}")
    include(CPack)
endfunction()
