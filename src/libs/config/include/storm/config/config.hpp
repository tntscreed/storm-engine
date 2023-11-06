#pragma once

#include "istring.hpp"

#include <toml++/toml.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <map>

namespace storm {

struct ConfigCompare {
    constexpr bool operator () (const std::string_view &first, const std::string_view &second) const noexcept {
        return traits_cast<ichar_traits<char>>(first) < traits_cast<ichar_traits<char>>(second);
    }
};

template<typename U, typename V, typename... Args>
using ConfigObject = std::map<U, V, ConfigCompare>;

using Config = nlohmann::basic_json<ConfigObject>;

enum ConfigFormat {
    Ini,
    Toml,
};

struct FindConfigResult
{
    ConfigFormat format;
    const std::filesystem::path path;
};

[[nodiscard]] std::optional<FindConfigResult> FindConfigFile(const std::filesystem::path &source_path);

[[nodiscard]] std::optional<Config> LoadConfig(const std::filesystem::path &file_path);

namespace config {

std::optional<uint32_t> GetColor(const Config &node);

} // namespace config

} // namespace storm
