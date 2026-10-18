##
## EPITECH PROJECT, 2026
## G-ING-500-COT-5-1-rtype-2
## File description:
## CMakeLists
##

# engine/plugins/platform_sfml/CMakeLists.txt
# ENG-31 — platform_sfml plugin: window, events to InputMap, batching renderer
# Spec: docs/engine/reference/08-rendering.md · ADR 0007, 0031
# Only Graphics (and Window, System through it): never SFML::Audio, the process has one miniaudio (ENG-35).
# No UI library here (Dear ImGui dropped on 2026-10-18): the Pulsar tools draw their UI with SFML 3 and
# reSf (engine/tools/editor), the in-game debug overlay goes through the RenderQueue (ENG-47).

find_package(SFML 3 COMPONENTS Graphics CONFIG REQUIRED)

engine_add_plugin(platform_sfml SOURCES SfmlPlatform.cpp SfmlRenderer.cpp SfmlKeys.cpp DEPENDS SFML::Graphics)
