##
## EPITECH PROJECT, 2026
## G-ING-500-COT-5-1-rtype-2
## File description:
## CMakeLists
##

find_path(MINIAUDIO_INCLUDE_DIR miniaudio.h REQUIRED)
find_package(Threads REQUIRED)

set_source_files_properties(MiniaudioImplementation.cpp PROPERTIES
    COMPILE_OPTIONS $<IF:$<CXX_COMPILER_ID:MSVC>,/W0,-w>)
engine_add_plugin(audio_miniaudio SOURCES MiniaudioAudio.cpp MiniaudioImplementation.cpp
                  DEPENDS Threads::Threads ${CMAKE_DL_LIBS})
target_include_directories(audio_miniaudio SYSTEM PRIVATE ${MINIAUDIO_INCLUDE_DIR})
if(UNIX)
    target_link_libraries(audio_miniaudio PRIVATE m)
endif()
