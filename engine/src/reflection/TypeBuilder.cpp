/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TypeBuilder
*/

#include "engine/reflection/TypeBuilder.hpp"

#include <string>
#include <string_view>

namespace engine {

namespace {

std::string_view trim(std::string_view text) {
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t' || text.front() == '\n'))
        text.remove_prefix(1);
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t' || text.back() == '\n'))
        text.remove_suffix(1);
    return text;
}

}

EnumInfo makeEnumInfo(std::string_view names, std::span<const int64_t> values) {
    EnumInfo info;
    size_t i = 0;
    while (!names.empty() && i < values.size()) {
        const size_t comma = names.find(',');
        const std::string_view name = trim(names.substr(0, comma));
        info.values.emplace_back(std::string(name), values[i++]);
        names = comma == std::string_view::npos ? std::string_view{} : names.substr(comma + 1);
    }
    return info;
}

}
