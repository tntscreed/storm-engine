#pragma once

#include <filesystem>
#include <optional>

#ifdef _WIN32 // SHGetKnownFolderPath
#include <ShlObj.h>
#else
#include <SDL2/SDL.h>
#endif

/* Filesystem proxy */
namespace fs
{
using namespace std::filesystem;

inline std::optional<path> GetDefaultStashPath()
{
    static std::filesystem::path path;

    if (path.empty())
    {
#ifdef _WIN32 // SHGetKnownFolderPath
        wchar_t *str = nullptr;
        if (SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_SIMPLE_IDLIST, nullptr, &str) != S_OK || str == nullptr) {
            return {};
        }
        path = std::filesystem::path(str) / "My Games" / "Sea Dogs";
        CoTaskMemFree(str);
#else
        char *pref_path = nullptr;
        pref_path = SDL_GetPrefPath("Akella", "Sea Dogs");
        if (pref_path == nullptr) {
            return {};
        }
        path = pref_path;
#endif
    }
    return path;
}

inline path GetStashPath()
{
    return GetDefaultStashPath().value_or(std::filesystem::current_path());
}

inline path GetLogsPath()
{
    return GetStashPath() / "Logs";
}

inline path GetSaveDataPath()
{
    return GetStashPath() / "SaveData";
}

inline path GetScreenshotsPath()
{
    return GetStashPath() / "Screenshots";
}

constexpr char ENGINE_INI_FILE_NAME[] = "engine.ini";
} // namespace fs
