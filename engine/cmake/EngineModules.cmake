# One-line CMake functions for plugins, game modules and launchers, in dynamic and static mode.
# ENG-02 · ADR 0005, 0009, 0020, 0040 · docs/engine/reference/02-modules.md
#
# Dynamic mode (ENGINE_DYNAMIC_MODULES=ON): plugins and games are shared libraries in bin/plugins and
# bin/games, loaded at runtime by ModuleManager. Static mode: they are static libraries, linked whole
# into each launcher so their registrars (ENGINE_PLUGIN / ENGINE_GAME_MODULE) fill StaticModuleTable.
# Declare plugins and games before the launchers that use them (the engine's plugins come first).

include_guard(GLOBAL)
include(GNUInstallDirs)
include(EngineOptions)

# The engine's main(); a test can point it to another file.
set(ENGINE_LAUNCHER_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../launcher/main.cpp" CACHE FILEPATH
    "Source of the generic launcher main()")

function(_engine_module_library target kind folder output_name)
    if(ENGINE_DYNAMIC_MODULES)
        add_library(${target} SHARED)
        set_target_properties(${target} PROPERTIES
            PREFIX "lib"                                            # libplatform_sfml.dll on Windows too
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${folder}
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${folder}
            BUILD_RPATH "$ORIGIN/.."                                # finds libengine.so in bin/
            INSTALL_RPATH "$ORIGIN/../../${CMAKE_INSTALL_LIBDIR}")   # installed: bin/<folder> → lib/
    else()
        add_library(${target} STATIC)
    endif()
    set_target_properties(${target} PROPERTIES OUTPUT_NAME ${output_name})
    target_link_libraries(${target} PUBLIC rtype_engine::engine)
    engine_configure_target(${target})
    set_property(TARGET ${target} PROPERTY ENGINE_MODULE_KIND ${kind})
    # Installed next to the executables (ENG-62). The engine's own plugins join its export set, so an
    # installed engine provides them as rtype_engine::<kind>_<name>.
    if(kind STREQUAL "plugin" AND ENGINE_BUILDING_ENGINE)
        install(TARGETS ${target} EXPORT rtype_engineTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/${folder}
            LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}/${folder}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    elseif(ENGINE_DYNAMIC_MODULES)
        install(TARGETS ${target}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/${folder}
            LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}/${folder})
    endif()
endfunction()

# engine_add_plugin(<kind>_<name> SOURCES ... [DEPENDS <targets>])
function(engine_add_plugin target)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "SOURCES;DEPENDS")
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "engine_add_plugin(${target}): SOURCES is required")
    endif()
    if(NOT target MATCHES "^(platform|audio|network|scripting)_[a-z0-9_]+$")
        message(FATAL_ERROR "engine_add_plugin(${target}): the name must be <kind>_<name>, "
                            "kind one of platform, audio, network, scripting (reference 02)")
    endif()
    _engine_module_library(${target} plugin plugins ${target})
    target_sources(${target} PRIVATE ${ARG_SOURCES})
    target_compile_definitions(${target} PRIVATE ENGINE_BUILDING_PLUGIN)
    if(ARG_DEPENDS)
        target_link_libraries(${target} PRIVATE ${ARG_DEPENDS})
    endif()
    set_property(GLOBAL APPEND PROPERTY ENGINE_PLUGIN_TARGETS ${target})
endfunction()

# engine_add_game_module(<name> [SHARED_SOURCES ...] [SERVER_SOURCES ...] [CLIENT_SOURCES ...]
#                        [SOURCES ...] [DEPENDS <targets>])
# Split game (ADR 0040): lib<name>_server = shared + server, lib<name>_client = shared + client.
# Simple game: SOURCES only, one lib<name> for every role.
# CMake targets are game_<name>_server / game_<name>_client (or game_<name>), so a launcher may be
# called <name>_client without clashing; the files keep the names ModuleManager loads.
function(engine_add_game_module name)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "SHARED_SOURCES;SERVER_SOURCES;CLIENT_SOURCES;SOURCES;DEPENDS")
    if(ARG_SOURCES AND (ARG_SERVER_SOURCES OR ARG_CLIENT_SOURCES OR ARG_SHARED_SOURCES))
        message(FATAL_ERROR "engine_add_game_module(${name}): use SOURCES alone, or SHARED/SERVER/CLIENT_SOURCES")
    endif()
    if(ARG_SOURCES)
        _engine_module_library(game_${name} game games ${name})
        target_sources(game_${name} PRIVATE ${ARG_SOURCES})
        if(ARG_DEPENDS)
            target_link_libraries(game_${name} PRIVATE ${ARG_DEPENDS})
        endif()
        set_property(GLOBAL PROPERTY ENGINE_GAME_${name}_TARGETS "game_${name}")
        return()
    endif()
    if(NOT ARG_SERVER_SOURCES OR NOT ARG_CLIENT_SOURCES)
        message(FATAL_ERROR "engine_add_game_module(${name}): SERVER_SOURCES and CLIENT_SOURCES are both required "
                            "(or use SOURCES for a game with one module)")
    endif()
    foreach(role IN ITEMS server client)
        string(TOUPPER ${role} ROLE)
        set(target game_${name}_${role})
        _engine_module_library(${target} game games ${name}_${role})
        target_sources(${target} PRIVATE ${ARG_SHARED_SOURCES} ${ARG_${ROLE}_SOURCES})
        target_compile_definitions(${target} PRIVATE ENGINE_GAME_${ROLE})
        if(ARG_DEPENDS)
            target_link_libraries(${target} PRIVATE ${ARG_DEPENDS})
        endif()
    endforeach()
    set_property(GLOBAL PROPERTY ENGINE_GAME_${name}_TARGETS "game_${name}_server;game_${name}_client")
endfunction()

# engine_add_launcher(<executable> CONFIG <file.yaml> [GAME <name>] [ROLE client|server|editor])
# ROLE picks the game build to link in static mode for a split game; the editor uses the client build.
function(engine_add_launcher exe)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "CONFIG;GAME;ROLE" "")
    if(NOT ARG_CONFIG)
        message(FATAL_ERROR "engine_add_launcher(${exe}): CONFIG <file.yaml> is required")
    endif()
    get_filename_component(config_path "${ARG_CONFIG}" ABSOLUTE BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    get_filename_component(config_name "${ARG_CONFIG}" NAME)

    add_executable(${exe} ${ENGINE_LAUNCHER_SOURCE})
    target_link_libraries(${exe} PRIVATE rtype_engine::engine)
    target_compile_definitions(${exe} PRIVATE ENGINE_DEFAULT_CONFIG="${config_name}")
    engine_configure_target(${exe})
    set_target_properties(${exe} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin                # next to libengine and the DLLs
        BUILD_RPATH "$ORIGIN" INSTALL_RPATH "$ORIGIN;$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
    install(TARGETS ${exe} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    # A game built against an installed engine ships that engine's library too (once per project).
    get_target_property(engine_imported rtype_engine::engine IMPORTED)
    get_property(engine_shipped GLOBAL PROPERTY ENGINE_RUNTIME_INSTALLED)
    if(engine_imported AND ENGINE_DYNAMIC_MODULES AND NOT engine_shipped)
        install(IMPORTED_RUNTIME_ARTIFACTS rtype_engine::engine
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
        set_property(GLOBAL PROPERTY ENGINE_RUNTIME_INSTALLED ON)
    endif()
    install(FILES "${config_path}" DESTINATION ${CMAKE_INSTALL_BINDIR})

    add_custom_command(TARGET ${exe} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${config_path}" "$<TARGET_FILE_DIR:${exe}>/${config_name}"
        COMMENT "Copying ${config_name} next to ${exe}")

    get_property(plugins GLOBAL PROPERTY ENGINE_PLUGIN_TARGETS)
    set(games "")
    if(ARG_GAME)
        get_property(games GLOBAL PROPERTY ENGINE_GAME_${ARG_GAME}_TARGETS)
        if(NOT games)
            message(FATAL_ERROR "engine_add_launcher(${exe}): game '${ARG_GAME}' is unknown; "
                                "call engine_add_game_module(${ARG_GAME} …) first")
        endif()
    endif()

    if(ENGINE_DYNAMIC_MODULES)
        # Built with the executable, loaded at runtime (an installed engine's plugins are already built).
        foreach(module IN LISTS plugins games)
            get_target_property(imported ${module} IMPORTED)
            if(NOT imported)
                add_dependencies(${exe} ${module})
            endif()
        endforeach()
        return()
    endif()

    # Static mode: link everything whole, so registrar objects are kept (reference 02).
    list(LENGTH games game_count)
    if(game_count GREATER 1)
        if(NOT ARG_ROLE)
            message(FATAL_ERROR "engine_add_launcher(${exe}): static mode needs ROLE client|server|editor "
                                "to pick lib${ARG_GAME}_client or lib${ARG_GAME}_server")
        endif()
        if(ARG_ROLE STREQUAL "server")
            set(games game_${ARG_GAME}_server)
        elseif(ARG_ROLE STREQUAL "client" OR ARG_ROLE STREQUAL "editor")
            set(games game_${ARG_GAME}_client)
        else()
            message(FATAL_ERROR "engine_add_launcher(${exe}): ROLE must be client, server or editor")
        endif()
    endif()
    foreach(module IN LISTS plugins games)
        target_link_libraries(${exe} PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,${module}>")
    endforeach()
endfunction()
