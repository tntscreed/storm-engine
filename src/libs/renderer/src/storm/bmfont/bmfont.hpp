#pragma once

#include "../../font.h"

#include <cstdint>
#include <string>
#include <vector>

namespace storm::bmfont {

class BmCharacter {
  public:
    char32_t id{};
    int16_t x{};
    int16_t y{};
    uint16_t width{};
    uint16_t height{};
    int16_t xoffset{};
    int16_t yoffset{};
    int16_t xadvance{};
    uint8_t page{};
    uint8_t channel{};
};

class BmKerning {
  public:
    char32_t first{};
    char32_t second{};
    int16_t amount{};
};

class BmTexture {
  public:
    std::string texturePath_;
    int32_t textureHandle_{};
};

class BmFont : public VFont {
  public:
    BmFont(const std::string_view &file_path, VDX9RENDER &renderer);
    ~BmFont() override = default;

    std::optional<size_t> Print(size_t x, size_t y, const std::string_view &text,
                                const FontPrintOverrides &overrides) override;
    size_t GetStringWidth(const std::string_view &text, const FontPrintOverrides &overrides) const override;
    size_t GetHeight() const override;
    void TempUnload() override;
    void RepeatInit() override;

  private:
    void LoadFromFnt(const std::string &file_path);

    std::vector<BmCharacter> characters_;
    std::vector<BmKerning> kerning_;
    std::vector<BmTexture> textures_;

    size_t lineHeight_{};
    size_t base_{};
    size_t textureWidth_{};
    size_t textureHeight_{};
};

} // namespace storm::bmfont
