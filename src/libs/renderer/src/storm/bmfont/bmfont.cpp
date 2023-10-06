#include "bmfont.hpp"

#include <array>
#include <fstream>

namespace storm::bmfont {

namespace {

bool IsSignatureValid(const std::array<char, 4> &signature) {
    return signature[0] == 'B' && signature[1] == 'M' && signature[2] == 'F' && signature[3] == 3;
}

template<typename T>
T Read(std::ifstream &stream)
{
    T value{};
    stream.read(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
}

std::string ReadString(std::ifstream &stream)
{
    std::string result{};
    result.resize(MAX_PATH);
    stream.get(reinterpret_cast<char *>(result.data()), result.size(), '\0');
    char next = stream.get();
    while (next != '\0') {
        size_t old_size = result.size();
        result.resize(old_size * 2);
        stream.get(reinterpret_cast<char *>(result.data() + old_size - 1), old_size, '\0');
        next = stream.get();
    }
    result.resize(strlen(result.c_str()));
    return result;
}

} // namespace

BmFont::BmFont(const std::string_view &file_path, VDX9RENDER &renderer)
{
    LoadFromFnt(std::string(file_path));
}

std::optional<size_t> BmFont::Print(size_t x, size_t y, const std::string_view &text,
                                    const FontPrintOverrides &overrides)
{
    return std::optional<size_t>();
}

size_t BmFont::GetStringWidth(const std::string_view &text, const FontPrintOverrides &overrides) const
{
    return 0;
}

size_t BmFont::GetHeight() const
{
    return 0;
}

void BmFont::TempUnload()
{
}

void BmFont::RepeatInit()
{
}

void BmFont::LoadFromFnt(const std::string &file_path)
{
    std::ifstream file(file_path, std::ios::binary);

    std::array<char, 4> signature{};
    file.read(signature.data(), signature.size());
    if (!IsSignatureValid(signature)) {
        throw std::runtime_error("Unsupported Bitmap Font file, signature check failed");
    }

    while(!file.eof()) {
        auto block_type = Read<int8_t>(file);
        auto block_size = Read<int32_t>(file);
        switch (block_type) {
        case 1: { // info
            file.ignore(2);
            auto bitfield = Read<int8_t>(file);
            if (!(bitfield & 0b0100'0000)) {
                throw std::runtime_error("Unsupported Bitmap Font file, not unicode");
            }
            file.ignore(block_size - 3);
            break;
        }
        case 2: { // common
            lineHeight_ = Read<int16_t>(file);
            base_ = Read<int16_t>(file);
            textureWidth_ = Read<int16_t>(file);
            textureHeight_ = Read<int16_t>(file);
            auto pages = Read<int16_t>(file);
            textures_.resize(pages);
            file.ignore(block_size - 10);
            break;
        }
        case 3: { // pages
            for (auto &texture : textures_) {
                texture.texturePath_ = ReadString(file);
            }
            break;
        }
        case 4: { // chars
            const auto count = block_size / 20;
            characters_.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                characters_.emplace_back(BmCharacter{
                    .id = Read<char32_t>(file),
                    .x = Read<int16_t>(file),
                    .y = Read<int16_t>(file),
                    .width = Read<uint16_t>(file),
                    .height = Read<uint16_t>(file),
                    .xoffset = Read<int16_t>(file),
                    .yoffset = Read<int16_t>(file),
                    .xadvance = Read<int16_t>(file),
                    .page = Read<uint8_t>(file),
                    .channel = Read<uint8_t>(file),
                });
            }
            break;
        }
        case 5: { // kerning pairs
            const auto count = block_size / 10;
            kerning_.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                kerning_.emplace_back(BmKerning{
                    .first = Read<char32_t>(file),
                    .second = Read<char32_t>(file),
                    .amount = Read<int16_t>(file),
                });
            }
            break;
        }
        default:
            file.ignore(block_size);
            break;
        }
    }
}

} // namespace storm::bmfont
