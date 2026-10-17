/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** InputMap
*/

#include "engine/input/InputMap.hpp"

#include "engine/reflection/Yaml.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <fstream>
#include <functional>
#include <utility>

namespace engine {

namespace {

constexpr std::string_view MousePrefix = "Mouse:";
constexpr std::string_view GamepadPrefix = "Gamepad:";
constexpr std::string_view AxisPrefix = "Axis:";

bool contains(const std::vector<ActionId>& list, ActionId a) {
    return std::ranges::find(list, a) != list.end();
}

Result<void> readSet(const YamlNode& node, std::string_view section,
                     const std::function<void(ActionId, std::string_view)>& name,
                     std::unordered_map<ActionId, std::vector<Binding>>& out, std::vector<ActionId>* order) {
    if (node.isNull()) return {};
    if (!node.isMap()) return node.error(Errc::ParseError, std::format("'{}' maps action names to bindings", section));
    for (const auto& [label, list] : node.entries()) {
        const ActionId id = action(label);
        name(id, label);
        if (order && !contains(*order, id)) order->push_back(id);
        std::vector<Binding>& bindings = out[id];
        bindings.clear();
        if (list.isNull()) continue;
        if (!list.isSequence())
            return list.error(Errc::ParseError, std::format("action '{}' expects a list of bindings", label));
        for (size_t i = 0; i < list.size(); ++i) {
            const auto binding = list[i].isScalar() ? parseBinding(list[i].scalar()) : std::nullopt;
            if (!binding)
                return list[i].error(Errc::ParseError,
                                     std::format("unknown binding {} for action '{}'", list[i].describe(), label));
            bindings.push_back(*binding);
        }
    }
    return {};
}

}

std::string bindingName(const Binding& binding) {
    if (const auto* key = std::get_if<Key>(&binding)) return std::string(keyName(*key));
    if (const auto* mouse = std::get_if<MouseButton>(&binding))
        return std::string(MousePrefix) + std::string(mouseButtonName(*mouse));
    if (const auto* button = std::get_if<GamepadButton>(&binding))
        return std::string(GamepadPrefix) + std::string(gamepadButtonName(*button));
    const auto& axis = std::get<AxisBinding>(binding);
    return std::format("{}{}{}", AxisPrefix, gamepadAxisName(axis.axis), axis.direction < 0 ? '-' : '+');
}

std::optional<Binding> parseBinding(std::string_view text) {
    if (text.starts_with(MousePrefix)) {
        if (auto mouse = mouseButtonFromName(text.substr(MousePrefix.size()))) return Binding{*mouse};
        return std::nullopt;
    }
    if (text.starts_with(GamepadPrefix)) {
        if (auto button = gamepadButtonFromName(text.substr(GamepadPrefix.size()))) return Binding{*button};
        return std::nullopt;
    }
    if (text.starts_with(AxisPrefix)) {
        std::string_view rest = text.substr(AxisPrefix.size());
        if (rest.empty() || (rest.back() != '-' && rest.back() != '+')) return std::nullopt;
        const float direction = rest.back() == '-' ? -1.f : 1.f;
        rest.remove_suffix(1);
        if (auto axis = gamepadAxisFromName(rest)) return Binding{AxisBinding{*axis, direction, 0.5f}};
        return std::nullopt;
    }
    if (auto key = keyFromName(text)) return Binding{*key};
    return std::nullopt;
}

void InputMap::remember(ActionId a, std::string_view label) {
    this->_names[a] = std::string(label);
}

InputMap::BindingSet& InputMap::setOf(ActionId a) {
    if (this->_options.oneHanded && this->_oneHanded.contains(a)) return this->_oneHanded;
    return this->_bindings;
}

void InputMap::refresh() {
    this->_active = this->_bindings;
    if (this->_options.oneHanded)
        for (const auto& [id, list] : this->_oneHanded)
            this->_active[id] = list;
    for (const auto& [id, list] : this->_active)
        this->_latches[id].down = this->isDown(id);
}

Result<void> InputMap::load(const std::filesystem::path& file) {
    auto root = YamlNode::load(file, file.generic_string());
    if (!root) return unexpected(root.error());
    if (!root->isMap())
        return root->error(Errc::ParseError, "input.yaml is a map with 'actions', 'one_handed', 'options'");
    BindingSet bindings;
    BindingSet oneHanded;
    std::vector<ActionId> order;
    InputOptions options;
    const auto name = [this](ActionId a, std::string_view label) { this->remember(a, label); };
    for (const auto& [key, value] : root->entries()) {
        if (key == "actions") {
            if (auto ok = readSet(value, key, name, bindings, &order); !ok) return ok;
        } else if (key == "one_handed") {
            if (auto ok = readSet(value, key, name, oneHanded, nullptr); !ok) return ok;
        } else if (key == "options") {
            if (!value.isMap()) return value.error(Errc::ParseError, "'options' is a map");
            for (const auto& [option, setting] : value.entries()) {
                if (option == "holdToToggle" || option == "autoRepeat") {
                    if (!setting.isSequence())
                        return setting.error(Errc::ParseError, std::format("'{}' expects a list of actions", option));
                    auto& list = option == "holdToToggle" ? options.holdToToggle : options.autoRepeat;
                    for (size_t i = 0; i < setting.size(); ++i)
                        list.push_back(action(setting[i].scalar()));
                } else if (option == "autoRepeatHz" || option == "deadzone") {
                    const auto number = setting.asDouble();
                    if (!number || *number < 0)
                        return setting.error(Errc::ParseError, std::format("'{}' expects a positive number", option));
                    (option == "autoRepeatHz" ? options.autoRepeatHz : options.deadzone) = static_cast<float>(*number);
                } else if (option == "oneHanded") {
                    const auto flag = setting.asBool();
                    if (!flag) return setting.error(Errc::ParseError, "'oneHanded' expects true or false");
                    options.oneHanded = *flag;
                } else {
                    return setting.error(Errc::ParseError, std::format("unknown input option '{}'", option));
                }
            }
        } else {
            return value.error(Errc::ParseError,
                               std::format("unknown key '{}' (expected 'actions', 'one_handed' or 'options')", key));
        }
    }
    this->_bindings = std::move(bindings);
    this->_oneHanded = std::move(oneHanded);
    this->_order = std::move(order);
    this->_options = std::move(options);
    this->refresh();
    return {};
}

Result<void> InputMap::save(const std::filesystem::path& file) const {
    const auto labelOf = [this](ActionId a) {
        const auto it = this->_names.find(a);
        return it != this->_names.end() ? it->second : std::format("#{:016x}", a.key);
    };
    const auto writeSet = [&](YamlWriter& out, const BindingSet& set, const std::vector<ActionId>& order) {
        out.beginMap();
        for (ActionId id : order) {
            const auto it = set.find(id);
            if (it == set.end()) continue;
            out.key(labelOf(id));
            out.beginSequence(true);
            for (const Binding& binding : it->second)
                out.value(bindingName(binding));
            out.endSequence();
        }
        out.endMap();
    };
    std::vector<ActionId> order = this->_order;
    for (const auto& [id, list] : this->_bindings)
        if (!contains(order, id)) order.push_back(id);
    std::vector<ActionId> handed;
    for (const auto& [id, list] : this->_oneHanded)
        handed.push_back(id);
    std::ranges::sort(handed, {}, [&](ActionId a) { return labelOf(a); });

    YamlWriter out;
    out.beginMap();
    out.key("actions");
    writeSet(out, this->_bindings, order);
    if (!handed.empty()) {
        out.key("one_handed");
        writeSet(out, this->_oneHanded, handed);
    }
    out.key("options");
    out.beginMap();
    for (const auto& [label, list] : {std::pair{"holdToToggle", &this->_options.holdToToggle},
                                      std::pair{"autoRepeat", &this->_options.autoRepeat}}) {
        out.key(label);
        out.beginSequence(true);
        for (ActionId id : *list)
            out.value(labelOf(id));
        out.endSequence();
    }
    out.key("autoRepeatHz");
    out.value(this->_options.autoRepeatHz);
    out.key("deadzone");
    out.value(this->_options.deadzone);
    out.key("oneHanded");
    out.value(this->_options.oneHanded);
    out.endMap();
    out.endMap();

    std::ofstream stream(file, std::ios::binary | std::ios::trunc);
    if (!stream) return fail(Errc::FileNotFound, "cannot write the input bindings", file.generic_string());
    stream << out.str() << '\n';
    return {};
}

void InputMap::name(ActionId a, std::string_view label) {
    this->remember(a, label);
    if (!contains(this->_order, a)) this->_order.push_back(a);
}

void InputMap::bind(ActionId a, Binding b) {
    auto& list = this->setOf(a)[a];
    if (std::ranges::find(list, b) == list.end()) list.push_back(b);
    if (!contains(this->_order, a)) this->_order.push_back(a);
    this->refresh();
}

void InputMap::unbind(ActionId a, const Binding& b) {
    auto& set = this->setOf(a);
    if (auto it = set.find(a); it != set.end()) std::erase(it->second, b);
    this->refresh();
}

void InputMap::clear(ActionId a) {
    this->setOf(a)[a].clear();
    this->refresh();
}

std::span<const Binding> InputMap::bindings(ActionId a) const {
    const auto it = this->_active.find(a);
    return it == this->_active.end() ? std::span<const Binding>{} : std::span<const Binding>(it->second);
}

std::optional<ActionId> InputMap::boundTo(const Binding& b) const {
    for (ActionId id : this->_order)
        if (std::ranges::find(this->bindings(id), b) != this->bindings(id).end()) return id;
    for (const auto& [id, list] : this->_active)
        if (std::ranges::find(list, b) != list.end()) return id;
    return std::nullopt;
}

void InputMap::setOptions(InputOptions o) {
    this->_options = std::move(o);
    this->refresh();
}

const InputOptions& InputMap::options() const {
    return this->_options;
}

float InputMap::level(const Binding& binding) const {
    if (const auto* key = std::get_if<Key>(&binding)) return this->_keys.contains(*key) ? 1.f : 0.f;
    if (const auto* mouse = std::get_if<MouseButton>(&binding)) return this->_mouse.contains(*mouse) ? 1.f : 0.f;
    if (const auto* button = std::get_if<GamepadButton>(&binding))
        return std::ranges::any_of(this->_buttons, [&](const auto& held) { return held.second == *button; }) ? 1.f
                                                                                                             : 0.f;
    const auto& axis = std::get<AxisBinding>(binding);
    float best = 0.f;
    for (const auto& [where, value] : this->_axes) {
        if (where.second != axis.axis || std::fabs(value) < this->_options.deadzone) continue;
        best = std::max(best, value * axis.direction);
    }
    return best;
}

bool InputMap::isDown(ActionId a) const {
    for (const Binding& binding : this->bindings(a)) {
        const float threshold =
            std::holds_alternative<AxisBinding>(binding) ? std::get<AxisBinding>(binding).threshold : 0.5f;
        if (this->level(binding) >= threshold) return true;
    }
    return false;
}

void InputMap::feed(const RawInputEvent& e) {
    using Type = RawInputEvent::Type;
    if (this->_capture) {
        std::optional<Binding> captured;
        if (e.type == Type::KeyDown && e.key != Key::Unknown) captured = e.key;
        if (e.type == Type::MouseDown) captured = e.mouse;
        if (e.type == Type::GamepadDown) captured = e.button;
        if (e.type == Type::GamepadAxis && std::fabs(e.value) >= 0.5f)
            captured = AxisBinding{e.axis, e.value < 0 ? -1.f : 1.f, 0.5f};
        if (captured) {
            auto capture = std::move(this->_capture);
            this->_capture = nullptr;
            capture(*captured);
            return;
        }
    }
    switch (e.type) {
    case Type::KeyDown:
        if (e.key != Key::Unknown) this->_keys.insert(e.key);
        break;
    case Type::KeyUp:
        this->_keys.erase(e.key);
        break;
    case Type::MouseDown:
        this->_mouse.insert(e.mouse);
        break;
    case Type::MouseUp:
        this->_mouse.erase(e.mouse);
        break;
    case Type::GamepadDown:
        this->_buttons.insert({e.device, e.button});
        break;
    case Type::GamepadUp:
        this->_buttons.erase({e.device, e.button});
        break;
    case Type::GamepadAxis:
        this->_axes[{e.device, e.axis}] = e.value;
        break;
    case Type::GamepadDisconnected:
        std::erase_if(this->_buttons, [&](const auto& held) { return held.first == e.device; });
        std::erase_if(this->_axes, [&](const auto& entry) { return entry.first.first == e.device; });
        break;
    default:
        return;
    }
    for (const auto& [id, list] : this->_active) {
        Latch& latch = this->_latches[id];
        const bool down = this->isDown(id);
        if (down && !latch.down) latch.wentDown = true;
        if (!down && latch.down) latch.wentUp = true;
        latch.down = down;
    }
}

void InputMap::beginTick(double dt) {
    const double period = this->_options.autoRepeatHz > 0 ? 1.0 / this->_options.autoRepeatHz : 0.0;
    for (const auto& [id, list] : this->_active) {
        Latch& latch = this->_latches[id];
        ActionState::Entry entry;
        if (contains(this->_options.holdToToggle, id)) {
            if (latch.wentDown) latch.toggled = !latch.toggled;
            entry.held = latch.toggled;
            entry.pressed = latch.wentDown && latch.toggled;
            entry.released = latch.wentDown && !latch.toggled;
        } else {
            entry.held = latch.down;
            entry.pressed = latch.wentDown;
            entry.released = latch.wentUp;
        }
        if (contains(this->_options.autoRepeat, id) && entry.held && period > 0) {
            if (entry.pressed) {
                latch.repeat = 0;
            } else {
                latch.repeat += dt;
                if (latch.repeat >= period) {
                    entry.pressed = true;
                    latch.repeat -= period;
                }
            }
        } else {
            latch.repeat = 0;
        }
        float axis = entry.held ? 1.f : 0.f;
        for (const Binding& binding : list)
            if (std::holds_alternative<AxisBinding>(binding)) axis = std::max(axis, this->level(binding));
        entry.axis = latch.toggled && contains(this->_options.holdToToggle, id) ? 1.f : axis;
        latch.wentDown = false;
        latch.wentUp = false;
        this->_state._actions[id] = entry;
    }
}

const ActionState& InputMap::state() const {
    return this->_state;
}

void InputMap::captureNextBinding(std::move_only_function<void(Binding)> onCaptured) {
    this->_capture = std::move(onCaptured);
}

}
