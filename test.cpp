#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tgafunc_cpp.h"

static void create_test(void) {
    using namespace tga;

    int size = 4;
    int oversize = TGA_MAX_IMAGE_DIMENSIONS + 1;

    // image size cannot be less than 1.
    {
        Image img(0, size, tga_pixel_format::TGA_PIXEL_RGB24);
        assert(img.last_error() ==
               tga_error::TGA_ERROR_INVALID_IMAGE_DIMENSIONS);
    }

    {
        Image img(size, 0, tga_pixel_format::TGA_PIXEL_RGB24);
        assert(img.last_error() ==
               tga_error::TGA_ERROR_INVALID_IMAGE_DIMENSIONS);
    }

    {
        Image img(-1, size, tga_pixel_format::TGA_PIXEL_RGB24);
        assert(img.last_error() ==
               tga_error::TGA_ERROR_INVALID_IMAGE_DIMENSIONS);
    }

    {
        Image img(size, -1, tga_pixel_format::TGA_PIXEL_RGB24);
        assert(img.last_error() ==
               tga_error::TGA_ERROR_INVALID_IMAGE_DIMENSIONS);
    }

    // Image size cannot be greater than TGA_MAX_IMAGE_DIMENSISNS.
    {
        Image img(oversize, size, tga_pixel_format::TGA_PIXEL_RGB24);
        assert(img.last_error() ==
               tga_error::TGA_ERROR_INVALID_IMAGE_DIMENSIONS);
    }

    {
        Image img(size, oversize, tga_pixel_format::TGA_PIXEL_RGB24);
        assert(img.last_error() ==
               tga_error::TGA_ERROR_INVALID_IMAGE_DIMENSIONS);
    }

    // Wrong pixel format check.
    {
        Image img(size, size, static_cast<tga_pixel_format>(100));
        assert(img.last_error() ==
               tga_error::TGA_ERROR_UNSUPPORTED_PIXEL_FORMAT);
    }

    // This time it should have succeeded.
    {
        Image img(size, size, tga_pixel_format::TGA_PIXEL_RGB24);
        assert(img.last_error() == tga_error::TGA_NO_ERROR);
        assert(img.get_width() == size);
        assert(img.get_height() == size);
        assert(img.get_pixel_format() == tga_pixel_format::TGA_PIXEL_RGB24);
        assert(img.get_pixel_size() == 3);
    }
}

static void load_test(void) {
    using namespace tga;

    const int image_size = 128;
    const char image_path[] = "D:/test/images/";
    const char* image_name_list[] = {
        "CBW8.TGA", "CCM8.TGA", "CTC16.TGA", "CTC24.TGA", "CTC32.TGA",
        "UBW8.TGA", "UCM8.TGA", "UTC16.TGA", "UTC24.TGA", "UTC32.TGA"};
    tga_pixel_format pixel_format_list[] = {
        tga_pixel_format::TGA_PIXEL_BW8,    tga_pixel_format::TGA_PIXEL_RGB555,
        tga_pixel_format::TGA_PIXEL_RGB555, tga_pixel_format::TGA_PIXEL_RGB24,
        tga_pixel_format::TGA_PIXEL_ARGB32, tga_pixel_format::TGA_PIXEL_BW8,
        tga_pixel_format::TGA_PIXEL_RGB555, tga_pixel_format::TGA_PIXEL_RGB555,
        tga_pixel_format::TGA_PIXEL_RGB24,  tga_pixel_format::TGA_PIXEL_ARGB32};

    char image_name[128];

    // Test the loading correctness of RLE images.
    int image_count = sizeof(image_name_list) / sizeof(image_name_list[0]);
    int group_size = image_count / 2;
    for (int i = 0; i < group_size; i++) {
        for (int j = 0; j < 2; j++) {
            int list_index = j * group_size + i;
            // Create file name.
            memcpy(image_name, image_path, sizeof(image_path));
            strcat(image_name, image_name_list[list_index]);
            // Load the image and check the image information.
            Image img(image_name);
            auto error_code = img.last_error();
            assert(error_code == tga_error::TGA_NO_ERROR);

            if (img.get_width() != image_size ||
                img.get_height() != image_size ||
                img.get_pixel_format() != pixel_format_list[list_index]) {
                // The loaded image information is wrong.
                assert(0);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    create_test();
    load_test();
    puts("Test cases passed.");
    return 0;
}