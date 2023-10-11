#include "font_loading.hpp"

#include "bmfont/bmfont.hpp"
#include "core.h"
#include "v_file_service.h"
#include <storm/config/config.hpp>

namespace storm {

std::unique_ptr<VFont> LoadFont(const std::string_view &font_name,
                                const std::string_view &ini_file_name,
                                VDX9RENDER &renderer,
                                IDirect3DDevice9 &device)
{
    const auto font_name_str = std::string(font_name);
    const auto ini_file_name_str = std::string(ini_file_name);

    const auto opt_config = storm::LoadConfig(ini_file_name);

    if (!opt_config) {
        return nullptr;
    }

    const auto &config = *opt_config;

    if (config.contains(font_name) && config[font_name].is_table() && config[font_name]["file"].is_string()) {
        const auto &section = config[font_name].as_table();
        auto msg = fmt::format("Loading BmFont {}", font_name);
        core.Trace(msg.c_str());
        std::string fnt_path = *section->at("file").value<std::string>();
        auto result = std::make_unique<bmfont::BmFont>(fnt_path, renderer);
        result->SetScale(section->at("pcscale").value_or<double>(1));
        return result;
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
