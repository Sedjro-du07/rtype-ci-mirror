##
## EPITECH PROJECT, 2026
## G-ING-500-COT-5-1-rtype-2
## File description:
## CMakeLists
##

find_package(asio CONFIG REQUIRED)

engine_add_plugin(network_asio SOURCES AsioNetwork.cpp DEPENDS asio::asio)
if(WIN32)
    target_compile_definitions(network_asio PRIVATE _WIN32_WINNT=0x0A00)
    target_link_libraries(network_asio PRIVATE ws2_32 mswsock)
endif()
