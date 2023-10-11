#pragma once

#include <toml++/toml.h>

#include <filesystem>

namespace storm {

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

[[nodiscard]] std::optional<toml::table> LoadConfig(const std::filesystem::path &file_path);

} // namespace storm
