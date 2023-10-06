#include "font_loading.hpp"

#include "core.h"
#include "v_file_service.h"

namespace storm {

std::unique_ptr<VFont> LoadFont(const std::string_view &font_name,
                                const std::string_view &ini_file_name,
                                VDX9RENDER &renderer,
                                IDirect3DDevice9 &device)
{
    const auto font_name_str = std::string(font_name);
    const auto ini_file_name_str = std::string(ini_file_name);
    auto ini = fio->OpenIniFile(ini_file_name_str.c_str());
    if (ini == nullptr)
        return nullptr;

    if (ini->TestKey(font_name_str.c_str(), "file", nullptr)) {
        auto msg = fmt::format("Loading BmFont {}", font_name);
        core.Trace(msg.c_str());
    }
    else {
        auto result = std::make_unique<FONT>(renderer, device);
        if (result->Init(font_name_str.c_str(), ini_file_name_str.c_str())) {
            return result;
        }
        else {
            core.Trace("Can't init font %s", font_name_str.c_str());
        }
    }

    return {};
}

} // namespace storm
