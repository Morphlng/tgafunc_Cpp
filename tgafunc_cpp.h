#pragma once

#include <vector>
#include <cstdint>
#include <string_view>

namespace tga
{

    ///
    /// \brief Image pixel format.
    ///
    /// The pixel data are all in little-endian. E.g. a TGA_PIXEL_ARGB32 format
    /// image, a single pixel is stored in the memory in the order of
    /// BBBBBBBB GGGGGGGG RRRRRRRR AAAAAAAA.
    ///
    enum class tga_pixel_format : uint8_t
    {
        ///
        /// \brief Single channel format represents grayscale, 8-bit integer.
        ///
        TGA_PIXEL_BW8,
        ///
        /// \brief Single channel format represents grayscale, 16-bit integer.
        ///
        TGA_PIXEL_BW16,
        ///
        /// \brief A 16-bit pixel format.
        /// The topmost bit is assumed to an attribute bit, usually ignored.
        /// Because of little-endian, this format pixel is stored in the memory in
        /// the order of GGGBBBBB ARRRRRGG.
        ///
        TGA_PIXEL_RGB555,
        ///
        /// \brief RGB color format, 8-bit per channel.
        ///
        TGA_PIXEL_RGB24,
        ///
        /// \brief RGB color with alpha format, 8-bit per channel.
        ///
        TGA_PIXEL_ARGB32
    };

    ///
    /// \brief Error code list.
    ///
    enum class tga_error : uint8_t
    {
        TGA_NO_ERROR = 0,
        TGA_ERROR_OUT_OF_MEMORY,
        TGA_ERROR_FILE_CANNOT_READ,
        TGA_ERROR_FILE_CANNOT_WRITE,
        TGA_ERROR_NO_DATA,
        TGA_ERROR_UNSUPPORTED_COLOR_MAP_TYPE,
        TGA_ERROR_UNSUPPORTED_IMAGE_TYPE,
        TGA_ERROR_UNSUPPORTED_PIXEL_FORMAT,
        TGA_ERROR_INVALID_IMAGE_DIMENSIONS,
        TGA_ERROR_COLOR_MAP_INDEX_FAILED
    };

    struct tga_info
    {
        uint16_t width, height;
        tga_pixel_format pixel_format;
    };

    class Image
    {
    public:
        Image(int width, int height, tga_pixel_format format);
        Image(std::string_view filepath);
        bool load(std::string_view filepath);
        bool save(std::string_view filename);

        void flip_h();
        void flip_v();

        uint8_t *get_pixel(int x, int y);
        uint8_t *get_raw_data();
        std::vector<uint8_t> &get_data();

        tga_error last_error() const;
        uint16_t get_width() const;
        uint16_t get_height() const;
        tga_pixel_format get_pixel_format() const;
        uint8_t get_pixel_size() const;
        const uint8_t *get_raw_data() const;
        const std::vector<uint8_t> &get_data() const;

    private:
        std::vector<uint8_t> data;
        tga_info img_info;
        tga_error err{tga_error::TGA_NO_ERROR};
    };
}