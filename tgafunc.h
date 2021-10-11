// Copyright (c) 2021 Caden Ji
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef TGAFUNC_H_
#define TGAFUNC_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

///
/// \brief Image pixel format.
/// The pixel data are all in little-endian. E.g. a TGA_PIXEL_ARGB32 format
/// image, a single pixel is stored in the memory in the order of
/// BBBBBBBB GGGGGGGG RRRRRRRR AAAAAAAA.
///
enum tga_pixel_format {
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
enum tga_error {
    TGA_NO_ERROR = 0,
    TGA_ERROR_OUT_OF_MEMORY,
    TGA_ERROR_FILE_CANNOT_READ,
    TGA_ERROR_FILE_CANNOT_WRITE,
    TGA_ERROR_NO_DATA,
    TGA_ERROR_UNSUPPORTED_COLOR_MAP_TYPE,
    TGA_ERROR_UNSUPPORTED_IMAGE_TYPE,
    TGA_ERROR_UNSUPPORTED_PIXEL_FORMAT,
    TGA_ERROR_INVALID_IMAGE_DIMENSISN
};

///
/// \brief Structure for saving TGA image information.
///
typedef struct tga_info tga_info;

///
/// \brief Creates a empty image.
/// The coordinates of the image start from the upper left corner. Image pixel
/// data is stored in a 1-dimensional array in a row major order.
/// ```
/// enum tga_error error_code;
/// uint8_t *image_data;
/// tga_info *image_info;
/// error_code = tga_create(&image_data, &image_info, 128, 128,
///                         TGA_PIXEL_RGB24);
/// if (error_code == TGA_NO_ERROR) {
///     // Use the created image.
/// }
/// ```
///
/// \param data_out Returns the image pixels data, all pixel values are set
///                 to 0x0. Uses tga_free_data() to release.
/// \param info_out Returns the information of the image. Uses tga_free_info()
///                 to release.
/// \param width The width of the image.
/// \param height The height of the image.
/// \param format The pixel data of the image.
/// \return enum tga_error The result of the creation.
///
enum tga_error tga_create(uint8_t **data_out, tga_info **info_out, int width,
                          int height, enum tga_pixel_format format);

///
/// \brief Loads TGA image from file.
/// The coordinates of the image start from the upper left corner. Image pixel
/// data is stored in a 1-dimensional array in a row major order.
/// ```
/// enum tga_error error_code;
/// uint8_t *image_data;
/// tga_info *image_info;
/// error_code = tga_load(&image_data, &image_info, file_name);
/// if (error_code == TGA_NO_ERROR) {
///     // Use the loaded image...
/// }
/// ```
///
/// \param data_out Returns the image pixels data. Uses tga_free_data() to
///                 release.
/// \param info_out Returns the information of the image. Uses tga_free_info()
///                 to release.
/// \param file_name The TGA image file name to be loaded.
/// \return enum tga_error The result of loading the image.
///
enum tga_error tga_load(uint8_t **data_out, tga_info **info_out,
                        const char *file_name);

///
/// \brief Saves TGA image to file.
/// If data or info is a null pointer, the function does nothing.
///
/// \param data The data of the image.
/// \param info The tga_info structure of the image.
/// \param file_name The name of the image file to be created.
/// \return enum tga_error he result of saving the image.
///
enum tga_error tga_save(const uint8_t *data, const tga_info *info,
                        const char *file_name);

///
/// \brief Gets the image width.
///
/// \param info The tga_info structure of the image.
/// \return int The width of the image.
///
int tga_get_image_width(const tga_info *info);

///
/// \brief Gets the image height.
///
/// \param info The tga_info structure of the image.
/// \return int The height of the image.
///
int tga_get_image_height(const tga_info *info);

///
/// \brief Gets the image pixel format.
///
/// \param info The tga_info structure of the image.
/// \return enum tga_pixel_format The pixel format of the image.
///
enum tga_pixel_format tga_get_pixel_format(const tga_info *info);

///
/// \brief Gets the number of bytes per pixel, this value is based on the pixel
///        format.
///
/// \param info The tga_info structure of the image.
/// \return uint8_t The number of bytes per pixel.
///
uint8_t tga_get_bytes_per_pixel(const tga_info *info);

///
/// \brief Releases the image data.
/// Releases the memory previously allocated by tga_load() or tga_create().
/// If the data is a null pointer, the function does nothing.
///
/// \param data The data to be released.
///
void tga_free_data(uint8_t *data);

///
/// \brief Releases the tga_info structure.
/// Releases the memory previously allocated by tga_load() or tga_create().
/// If the info is a null pointer, the function does nothing.
///
/// \param info The tga_info structure to be released.
///
void tga_free_info(tga_info *info);

///
/// \brief Flips the image horizontally.
/// Rearranges the order of pixels in data. If data or info is a null pointer,
/// the function does nothing.
///
/// \param data The image data to be flipped.
/// \param info The structure which contains the image information.
///
void tga_image_flip_h(uint8_t *data, const tga_info *info);

///
/// \brief Flip the image vertically.
/// Rearranges the order of pixels in data. If data or info is a null pointer,
/// the function does nothing.
///
/// \param data The image data to be flipped.
/// \param info The structure which contains the image information.
///
void tga_image_flip_v(uint8_t *data, const tga_info *info);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  // TGAFUNC_H_
