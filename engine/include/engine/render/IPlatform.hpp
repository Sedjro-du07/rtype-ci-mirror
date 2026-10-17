/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** IPlatform
*/

#pragma once

#include "engine/core/Error.hpp"
#include "engine/module/IPlugin.hpp"
#include "engine/render/IRenderer.hpp"
#include "engine/render/WindowDesc.hpp"

#include <string>
#include <string_view>

namespace engine {

class InputMap;
class EventBus;

class IPlatform : public IPlugin {
public:
    virtual Result<void> openWindow(const WindowDesc& desc) = 0;
    virtual void closeWindow() = 0;
    virtual bool windowOpen() const = 0;
    virtual void pollEvents(InputMap& input, EventBus& events) = 0;
    virtual IRenderer& renderer() = 0;
    virtual void setWindow(const WindowDesc& desc) = 0;
    virtual void beginDebugUi() = 0;
    virtual void endDebugUi() = 0;
    virtual void setClipboard(std::string_view text) = 0;
    virtual std::string clipboard() const = 0;
};

}
