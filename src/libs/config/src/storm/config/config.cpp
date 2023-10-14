#include "storm/config/config.hpp"
#include "v_file_service.h"

namespace storm
{

namespace
{

[[nodiscard]] ConfigFormat GetPreferredConfigFormat(const std::filesystem::path &source_path)
{
    if (source_path.has_extension())
    {
        const std::string extension = source_path.extension().string();
        if (extension == ".toml")
        {
            return ConfigFormat::Toml;
        }
        else if (extension == ".ini")
        {
            return ConfigFormat::Ini;
        }
    }

    return ConfigFormat::Toml;
}

std::string_view GetConfigFileExtension(ConfigFormat format)
{
    switch (format)
    {
    case Toml:
        return ".toml";
    case Ini:
        return ".ini";
    default:
        return "";
    }
}

[[nodiscard]] std::filesystem::path GetConfigFilePath(const std::filesystem::path &source_path, ConfigFormat format)
{
    const std::filesystem::path stem = source_path.stem();
    const std::filesystem::path directory = source_path.parent_path();

    return directory / (stem.string() + std::string(GetConfigFileExtension(format)));
}

} // namespace

std::optional<FindConfigResult> FindConfigFile(const std::filesystem::path &source_path)
{
    ConfigFormat preferred_format = GetPreferredConfigFormat(source_path);

    const std::filesystem::path first_test_path = GetConfigFilePath(source_path, preferred_format);
    if (exists(first_test_path))
    {
        return FindConfigResult{
            preferred_format,
            first_test_path,
        };
    }

    for (const auto format : {ConfigFormat::Toml, ConfigFormat::Ini})
    {
        if (format == preferred_format)
        {
            continue;
        }
        const std::filesystem::path test_path = GetConfigFilePath(source_path, format);
        if (exists(test_path))
        {
            return FindConfigResult{
                format,
                test_path,
            };
        }
    }

    return {};
}

std::optional<toml::table> LoadConfig(const std::filesystem::path &file_path)
{
    auto found_config = FindConfigFile(file_path);
    if (!found_config)
    {
        return {};
    }

    const auto &[format, path] = *found_config;

    switch (format)
    {
    case Toml: {
        return toml::parse_file(path.string());
    }
    case Ini: {
        const std::string path_str = path.string();
        const auto ini = fio->OpenIniFile(path_str.c_str());
        if (ini == nullptr)
            return {};
        return ini->ToToml();
    }
    default:
        return {};
    }
}

namespace config {

std::optional<uint32_t> GetColor(const toml::node_view<const toml::node> &node)
{
    if (node.is_integer()) {
        return node.value<uint32_t>();
    }
    else if (node.is_string()) {
        const std::string value = *node.value<std::string>();
        if (value.starts_with('#') && value.length() == 7) {
            const auto code = std::string_view(value.begin() + 1, value.end());
            uint32_t result;
            const char *first = code.data();
            const char *last = first + code.length();
            const auto parse_result = std::from_chars(first, last, result, 16);
            if (parse_result.ptr == last) {
                return result | 0xff000000;
            }
        }
    }
    return {};
}

} // namespace config

} // namespace storm
