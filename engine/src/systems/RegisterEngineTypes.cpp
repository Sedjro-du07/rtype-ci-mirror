/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RegisterEngineTypes
*/

#include "systems/RegisterEngineTypes.hpp"

#include "engine/assets/AssetManager.hpp"
#include "engine/components/Animation.hpp"
#include "engine/components/Collider.hpp"
#include "engine/components/Hierarchy.hpp"
#include "engine/components/Identity.hpp"
#include "engine/components/Lifetime.hpp"
#include "engine/components/Parallax.hpp"
#include "engine/components/Position.hpp"
#include "engine/components/Script.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/components/Text.hpp"
#include "engine/components/Velocity.hpp"
#include "engine/core/Hash.hpp"
#include "engine/events/EngineEvents.hpp"
#include "engine/reflection/TypeRegistry.hpp"
#include "engine/reflection/Yaml.hpp"

#include <format>
#include <string>

namespace engine {

namespace {

Result<void> readUuid(void* object, const YamlNode& node, LoadContext&) {
    const auto uuid = node.isScalar() ? parseUuid(node.scalar()) : std::nullopt;
    if (!uuid)
        return node.error(Errc::ParseError, std::format("Uuid expects hexadecimal digits, got {}", node.describe()));
    *static_cast<Uuid*>(object) = *uuid;
    return {};
}

void writeUuid(const void* object, YamlWriter& out, SaveContext&) {
    out.value(formatUuid(*static_cast<const Uuid*>(object)));
}

Result<void> readTags(void* object, const YamlNode& node, LoadContext&) {
    auto& tags = *static_cast<Tags*>(object);
    if (node.isNull()) {
        tags.ids.clear();
        return {};
    }
    if (!node.isSequence())
        return node.error(Errc::ParseError, std::format("Tags expects a list of labels, got {}", node.describe()));
    tags.ids.clear();
    for (size_t i = 0; i < node.size(); ++i) {
        const YamlNode label = node[i];
        if (!label.isScalar() || label.scalar().empty())
            return label.error(Errc::ParseError, std::format("a tag is a label, got {}", label.describe()));
        const std::string text = label.scalar();
        if (text.starts_with('#')) {
            const auto id = parseUuid(std::string_view(text).substr(1));
            if (!id || id->hi != 0) return label.error(Errc::ParseError, std::format("'{}' is not a tag id", text));
            tags.ids.push_back(id->lo);
        } else {
            tags.ids.push_back(fnv1a64(text));
        }
    }
    return {};
}

void writeTags(const void* object, YamlWriter& out, SaveContext&) {
    out.beginSequence(true);
    for (uint64_t id : static_cast<const Tags*>(object)->ids)
        out.value(std::format("#{:016x}", id));
    out.endSequence();
}

Result<void> readScript(void* object, const YamlNode& node, LoadContext& ctx) {
    auto& script = *static_cast<Script*>(object);
    if (node.isNull()) return {};
    YamlNode file = node.isScalar() ? node : node.get("file");
    if (node.isMap())
        for (const auto& [key, value] : node.entries())
            if (key != "file")
                return value.error(Errc::ParseError, std::format("unknown field '{}' in Script (only 'file')", key));
    if (!file.isScalar() || file.scalar().empty())
        return node.error(Errc::ParseError, std::format("Script expects a file path, got {}", node.describe()));
    script.file = file.scalar();
    if (!ctx.assets) return file.error(Errc::AssetLoadFailed, "Script needs the asset manager");
    script.handle = ctx.assets->script(script.file);
    return {};
}

void writeScript(const void* object, YamlWriter& out, SaveContext&) {
    out.beginMap(true);
    out.key("file");
    out.value(std::string_view(static_cast<const Script*>(object)->file));
    out.endMap();
}

}

void registerEngineTypes(TypeRegistry& types) {
    types.add<Name>().field("value", &Name::value, "label used by Registry::find and the console");
    types.add<Uuid>().flags(EditorHidden).yaml(readUuid, writeUuid);
    types.add<Tags>().yaml(readTags, writeTags);
    types.add<Parent>().field("value", &Parent::value).field("offset", &Parent::offset);
    types.add<Children>().field("list", &Children::list).flags(Transient);

    types.add<Position>().field("x", &Position::x).field("y", &Position::y).flags(Replicated);
    types.add<PreviousPosition>()
        .field("x", &PreviousPosition::x)
        .field("y", &PreviousPosition::y)
        .flags(Transient | EditorHidden);
    types.add<Velocity>().field("dx", &Velocity::dx).field("dy", &Velocity::dy).flags(Replicated);
    types.add<Sprite>()
        .field("texture", &Sprite::texture)
        .field("frame", &Sprite::frame)
        .field("layer", &Sprite::layer)
        .field("tint", &Sprite::tint)
        .field("scale", &Sprite::scale)
        .field("origin", &Sprite::origin)
        .field("rotation", &Sprite::rotation)
        .field("visible", &Sprite::visible)
        .flags(Replicated);
    types.add<Animation>()
        .field("frames", &Animation::frames)
        .field("fps", &Animation::fps)
        .field("loop", &Animation::loop)
        .field("time", &Animation::time)
        .field("current", &Animation::current);
    types.add<Text>()
        .field("value", &Text::value)
        .field("font", &Text::font)
        .field("size", &Text::size)
        .field("color", &Text::color)
        .field("layer", &Text::layer)
        .field("screenSpace", &Text::screenSpace);
    types.add<Collider>()
        .field("size", &Collider::size)
        .field("offset", &Collider::offset)
        .field("layer", &Collider::layer)
        .field("mask", &Collider::mask)
        .field("trigger", &Collider::trigger);
    types.add<Lifetime>().field("seconds", &Lifetime::seconds);
    types.add<Parallax>().field("factor", &Parallax::factor).field("wrapWidth", &Parallax::wrapWidth);
    types.add<Script>().field("file", &Script::file).yaml(readScript, writeScript);

    types.add<QuitRequested>();
    types.add<WindowResized>().field("width", &WindowResized::width).field("height", &WindowResized::height);
    types.add<WindowFocusChanged>().field("focused", &WindowFocusChanged::focused);
    types.add<AppSuspended>();
    types.add<AppResumed>();
    types.add<Collision>().field("a", &Collision::a).field("b", &Collision::b);
    types.add<SceneLoaded>().field("name", &SceneLoaded::name);
    types.add<SceneUnloading>().field("name", &SceneUnloading::name);
    types.add<Caption>().field("text", &Caption::text).field("seconds", &Caption::seconds);
    types.add<ConsoleCommand>().field("line", &ConsoleCommand::line);

    types.add<DisconnectReason>();
    types.add<PlayerJoined>().field("player", &PlayerJoined::player);
    types.add<PlayerLeft>()
        .field("player", &PlayerLeft::player)
        .field("reason", &PlayerLeft::reason)
        .field("message", &PlayerLeft::message);
    types.add<PlayerDisconnected>()
        .field("player", &PlayerDisconnected::player)
        .field("reason", &PlayerDisconnected::reason);
    types.add<PlayerReconnected>().field("player", &PlayerReconnected::player);
    types.add<JoinedServer>().field("me", &JoinedServer::me);
    types.add<LeftServer>().field("reason", &LeftServer::reason).field("message", &LeftServer::message);
    types.add<ConnectionInterrupted>().field("reason", &ConnectionInterrupted::reason);
    types.add<Reconnected>();
}

}
