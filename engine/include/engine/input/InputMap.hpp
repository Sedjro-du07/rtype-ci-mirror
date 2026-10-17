/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** InputMap
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/core/Error.hpp"
#include "engine/input/Action.hpp"
#include "engine/input/ActionState.hpp"
#include "engine/input/Keys.hpp"
#include "engine/input/RawInputEvent.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace engine {

struct AxisBinding {
    GamepadAxis axis = GamepadAxis::LeftX;
    float direction = 1;
    float threshold = 0.5f;

    bool operator==(const AxisBinding&) const = default;
};

using Binding = std::variant<Key, MouseButton, GamepadButton, AxisBinding>;

struct InputOptions {
    std::vector<ActionId> holdToToggle;
    std::vector<ActionId> autoRepeat;
    float autoRepeatHz = 8.f;
    float deadzone = 0.2f;
    bool oneHanded = false;
};

ENGINE_API std::string bindingName(const Binding& binding);
ENGINE_API std::optional<Binding> parseBinding(std::string_view text);

class ENGINE_API InputMap {
private:
    using BindingSet = std::unordered_map<ActionId, std::vector<Binding>>;

    struct Latch {
        bool down = false;
        bool wentDown = false;
        bool wentUp = false;
        bool toggled = false;
        double repeat = 0;
    };

    BindingSet _bindings;
    BindingSet _oneHanded;
    BindingSet _active;
    std::unordered_map<ActionId, std::string> _names;
    std::vector<ActionId> _order;
    InputOptions _options;
    std::set<Key> _keys;
    std::set<MouseButton> _mouse;
    std::set<std::pair<uint8_t, GamepadButton>> _buttons;
    std::map<std::pair<uint8_t, GamepadAxis>, float> _axes;
    std::unordered_map<ActionId, Latch> _latches;
    ActionState _state;
    std::move_only_function<void(Binding)> _capture;

    BindingSet& setOf(ActionId a);
    float level(const Binding& binding) const;
    bool isDown(ActionId a) const;
    void remember(ActionId a, std::string_view name);
    void refresh();

public:
    Result<void> load(const std::filesystem::path& file);
    Result<void> save(const std::filesystem::path& file) const;
    void name(ActionId a, std::string_view label);
    void bind(ActionId a, Binding b);
    void unbind(ActionId a, const Binding& b);
    void clear(ActionId a);
    std::span<const Binding> bindings(ActionId a) const;
    std::optional<ActionId> boundTo(const Binding& b) const;
    void setOptions(InputOptions o);
    const InputOptions& options() const;

    void feed(const RawInputEvent& e);
    void beginTick(double dt);
    const ActionState& state() const;
    void captureNextBinding(std::move_only_function<void(Binding)> onCaptured);
};

}
