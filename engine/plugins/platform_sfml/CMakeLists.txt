##
## EPITECH PROJECT, 2026
## G-ING-500-COT-5-1-rtype-2
## File description:
## CMakeLists
##

find_package(SFML 3 COMPONENTS Graphics CONFIG REQUIRED)

engine_add_plugin(platform_sfml SOURCES SfmlPlatform.cpp SfmlRenderer.cpp SfmlKeys.cpp DEPENDS SFML::Graphics)
