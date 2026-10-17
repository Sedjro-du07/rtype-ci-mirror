/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Yaml
*/

#include "engine/reflection/Yaml.hpp"

#include <yaml-cpp/yaml.h>

#include <format>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>

namespace engine {

struct YamlNode::Impl {
    YAML::Node node;
    std::shared_ptr<const std::string> file;
};

struct YamlWriter::Impl {
    YAML::Emitter out;
};

namespace {

template <class T>
std::optional<T> convert(const YAML::Node& node) {
    if (!node.IsDefined() || !node.IsScalar()) return std::nullopt;
    try {
        return node.as<T>();
    } catch (const YAML::Exception&) {
        return std::nullopt;
    }
}

}

YamlNode::YamlNode() = default;

YamlNode::YamlNode(std::shared_ptr<const Impl> impl) : _impl(std::move(impl)) {}

Result<YamlNode> YamlNode::parse(std::string_view text, std::string file) {
    try {
        auto impl = std::make_shared<Impl>();
        impl->node = YAML::Load(std::string(text));
        impl->file = std::make_shared<const std::string>(std::move(file));
        return YamlNode(std::move(impl));
    } catch (const YAML::Exception& error) {
        return fail(Errc::ParseError, error.msg, std::move(file), error.mark.line + 1);
    }
}

Result<YamlNode> YamlNode::load(const std::filesystem::path& path, std::string file) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return fail(Errc::FileNotFound, "cannot open file", std::move(file));
    const std::string text{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    return YamlNode::parse(text, std::move(file));
}

bool YamlNode::defined() const {
    return this->_impl && this->_impl->node.IsDefined();
}

bool YamlNode::isNull() const {
    return !this->defined() || this->_impl->node.IsNull();
}

bool YamlNode::isScalar() const {
    return this->defined() && this->_impl->node.IsScalar();
}

bool YamlNode::isSequence() const {
    return this->defined() && this->_impl->node.IsSequence();
}

bool YamlNode::isMap() const {
    return this->defined() && this->_impl->node.IsMap();
}

size_t YamlNode::size() const {
    return this->isSequence() || this->isMap() ? this->_impl->node.size() : 0;
}

YamlNode YamlNode::operator[](size_t index) const {
    if (!this->isSequence() || index >= this->size()) return {};
    const YAML::Node& node = this->_impl->node;
    return YamlNode(std::make_shared<const Impl>(Impl{node[index], this->_impl->file}));
}

YamlNode YamlNode::get(std::string_view key) const {
    if (!this->isMap()) return {};
    const YAML::Node& node = this->_impl->node;
    const YAML::Node child = node[std::string(key)];
    if (!child.IsDefined()) return {};
    return YamlNode(std::make_shared<const Impl>(Impl{child, this->_impl->file}));
}

std::vector<std::pair<std::string, YamlNode>> YamlNode::entries() const {
    std::vector<std::pair<std::string, YamlNode>> result;
    if (!this->isMap()) return result;
    for (const auto& entry : this->_impl->node) {
        auto impl = std::make_shared<const Impl>(Impl{entry.second, this->_impl->file});
        result.emplace_back(entry.first.Scalar(), YamlNode(std::move(impl)));
    }
    return result;
}

std::string YamlNode::scalar() const {
    return this->isScalar() ? this->_impl->node.Scalar() : std::string{};
}

std::optional<bool> YamlNode::asBool() const {
    return this->defined() ? convert<bool>(this->_impl->node) : std::nullopt;
}

std::optional<int64_t> YamlNode::asInt() const {
    return this->defined() ? convert<int64_t>(this->_impl->node) : std::nullopt;
}

std::optional<uint64_t> YamlNode::asUInt() const {
    if (!this->defined() || this->scalar().starts_with('-')) return std::nullopt;
    return convert<uint64_t>(this->_impl->node);
}

std::optional<double> YamlNode::asDouble() const {
    return this->defined() ? convert<double>(this->_impl->node) : std::nullopt;
}

std::string_view YamlNode::file() const {
    return this->_impl && this->_impl->file ? std::string_view(*this->_impl->file) : std::string_view{};
}

int YamlNode::line() const {
    if (!this->_impl) return 0;
    const YAML::Mark mark = this->_impl->node.Mark();
    return mark.line < 0 ? 0 : mark.line + 1;
}

std::string YamlNode::describe() const {
    if (!this->defined()) return "nothing";
    if (this->isNull()) return "null";
    if (this->isSequence()) return "a list";
    if (this->isMap()) return "a map";
    return std::format("\"{}\"", this->scalar());
}

unexpected<Error> YamlNode::error(Errc code, std::string message) const {
    return fail(code, std::move(message), std::string(this->file()), this->line());
}

YamlWriter::YamlWriter() : _impl(std::make_unique<Impl>()) {}

YamlWriter::~YamlWriter() = default;

void YamlWriter::beginMap(bool flow) {
    if (flow) this->_impl->out << YAML::Flow;
    this->_impl->out << YAML::BeginMap;
}

void YamlWriter::endMap() {
    this->_impl->out << YAML::EndMap;
}

void YamlWriter::beginSequence(bool flow) {
    if (flow) this->_impl->out << YAML::Flow;
    this->_impl->out << YAML::BeginSeq;
}

void YamlWriter::endSequence() {
    this->_impl->out << YAML::EndSeq;
}

void YamlWriter::key(std::string_view name) {
    this->_impl->out << YAML::Key << std::string(name) << YAML::Value;
}

void YamlWriter::value(std::string_view text) {
    this->_impl->out << std::string(text);
}

void YamlWriter::value(const char* text) {
    this->value(std::string_view(text));
}

void YamlWriter::value(bool flag) {
    this->_impl->out << flag;
}

void YamlWriter::value(int64_t number) {
    this->_impl->out << number;
}

void YamlWriter::value(uint64_t number) {
    this->_impl->out << number;
}

void YamlWriter::value(double number) {
    this->_impl->out << std::format("{}", number);
}

void YamlWriter::value(float number) {
    this->_impl->out << std::format("{}", number);
}

void YamlWriter::null() {
    this->_impl->out << YAML::Null;
}

std::string YamlWriter::str() const {
    return this->_impl->out.c_str();
}

}
