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
#include <utility>

namespace engine {

namespace {

constexpr std::string_view MousePrefix = "Mouse:";
constexpr std::string_view GamepadPrefix = "Gamepad:";
constexpr std::string_view AxisPrefix = "Axis:";

bool contains(const std::vector<ActionId>& list, ActionId a) {
    return std::ranges::find(list, a) != list.end();
}

using Bindings = std::unordered_map<ActionId, std::vector<Binding>>;

struct ParsedInput {
    Bindings bindings;
    Bindings oneHanded;
    std::vector<ActionId> order;
    std::vector<std::pair<ActionId, std::string>> names;
    InputOptions options;
};

Result<void> readBindings(const YamlNode& list, std::string_view label, std::vector<Binding>& out) {
    out.clear();
    if (list.isNull()) return {};
    if (!list.isSequence())
        return list.error(Errc::ParseError, std::format("action '{}' expects a list of bindings", label));
    for (size_t i = 0; i < list.size(); ++i) {
        const auto binding = list[i].isScalar() ? parseBinding(list[i].scalar()) : std::nullopt;
        if (!binding)
            return list[i].error(Errc::ParseError,
                                 std::format("unknown binding {} for action '{}'", list[i].describe(), label));
        out.push_back(*binding);
    }
    return {};
}

Result<void> readSet(const YamlNode& node, std::string_view section, ParsedInput& parsed, Bindings& out, bool ordered) {
    if (node.isNull()) return {};
    if (!node.isMap()) return node.error(Errc::ParseError, std::format("'{}' maps action names to bindings", section));
    for (const auto& [label, list] : node.entries()) {
        const ActionId id = action(label);
        parsed.names.emplace_back(id, label);
        if (ordered && !contains(parsed.order, id)) parsed.order.push_back(id);
        if (auto ok = readBindings(list, label, out[id]); !ok) return ok;
    }
    return {};
}

Result<void> readActionList(const YamlNode& node, std::string_view option, std::vector<ActionId>& out) {
    if (!node.isSequence()) return node.error(Errc::ParseError, std::format("'{}' expects a list of actions", option));
    for (size_t i = 0; i < node.size(); ++i)
        out.push_back(action(node[i].scalar()));
    return {};
}

Result<void> readPositive(const YamlNode& node, std::string_view option, float& out) {
    const auto number = node.asDouble();
    if (!number || *number < 0)
        return node.error(Errc::ParseError, std::format("'{}' expects a positive number", option));
    out = static_cast<float>(*number);
    return {};
}

Result<void> readFlag(const YamlNode& node, std::string_view option, bool& out) {
    const auto flag = node.asBool();
    if (!flag) return node.error(Errc::ParseError, std::format("'{}' expects true or false", option));
    out = *flag;
    return {};
}

Result<void> readOption(std::string_view option, const YamlNode& node, InputOptions& options) {
    if (option == "holdToToggle") return readActionList(node, option, options.holdToToggle);
    if (option == "autoRepeat") return readActionList(node, option, options.autoRepeat);
    if (option == "autoRepeatHz") return readPositive(node, option, options.autoRepeatHz);
    if (option == "deadzone") return readPositive(node, option, options.deadzone);
    if (option == "oneHanded") return readFlag(node, option, options.oneHanded);
    return node.error(Errc::ParseError, std::format("unknown input option '{}'", option));
}

Result<void> readOptions(const YamlNode& node, InputOptions& options) {
    if (!node.isMap()) return node.error(Errc::ParseError, "'options' is a map");
    for (const auto& [option, setting] : node.entries())
        if (auto ok = readOption(option, setting, options); !ok) return ok;
    return {};
}

Result<void> readSection(std::string_view key, const YamlNode& node, ParsedInput& parsed) {
    if (key == "actions") return readSet(node, key, parsed, parsed.bindings, true);
    if (key == "one_handed") return readSet(node, key, parsed, parsed.oneHanded, false);
    if (key == "options") return readOptions(node, parsed.options);
    return node.error(Errc::ParseError,
                      std::format("unknown key '{}' (expected 'actions', 'one_handed' or 'options')", key));
}

Result<ParsedInput> parseInput(const YamlNode& root) {
    if (!root.isMap())
        return root.error(Errc::ParseError, "input.yaml is a map with 'actions', 'one_handed', 'options'");
    ParsedInput parsed;
    for (const auto& [key, value] : root.entries())
        if (auto ok = readSection(key, value, parsed); !ok) return unexpected(ok.error());
    return parsed;
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
    auto parsed = parseInput(*root);
    if (!parsed) return unexpected(parsed.error());
    for (const auto& [id, label] : parsed->names)
        this->remember(id, label);
    this->_bindings = std::move(parsed->bindings);
    this->_oneHanded = std::move(parsed->oneHanded);
    this->_order = std::move(parsed->order);
    this->_options = std::move(parsed->options);
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

std::span<const ActionId> InputMap::order() const {
    return this->_order;
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
