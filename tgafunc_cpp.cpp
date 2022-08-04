#include "tgafunc_cpp.h"

#include <cstring>
#include <fstream>

// ----------------------Utilities----------------------

#define TGA_MAX_IMAGE_DIMENSIONS 65535

enum tga_image_type {
    TGA_TYPE_NO_DATA = 0,
    TGA_TYPE_COLOR_MAPPED = 1,
    TGA_TYPE_TRUE_COLOR = 2,
    TGA_TYPE_GRAYSCALE = 3,
    TGA_TYPE_RLE_COLOR_MAPPED = 9,
    TGA_TYPE_RLE_TRUE_COLOR = 10,
    TGA_TYPE_RLE_GRAYSCALE = 11
};

struct tga_header {
    uint8_t id_length;
    uint8_t map_type;
    uint8_t image_type;

    // Color map specification.
    uint16_t map_first_entry;
    uint16_t map_length;
    uint8_t map_entry_size;

    // Image specification.
    uint16_t image_x_origin;
    uint16_t image_y_origin;
    uint16_t image_width;
    uint16_t image_height;
    uint8_t pixel_depth;
    uint8_t image_descriptor;
};

struct color_map {
    uint16_t first_index{0};
    uint16_t entry_count{0};
    uint8_t bytes_per_entry{0};
    std::vector<uint8_t> pixels;
};

#define HEADER_SIZE 18

#define IS_SUPPORTED_IMAGE_TYPE(header)                  \
    ((header).image_type == TGA_TYPE_COLOR_MAPPED ||     \
     (header).image_type == TGA_TYPE_TRUE_COLOR ||       \
     (header).image_type == TGA_TYPE_GRAYSCALE ||        \
     (header).image_type == TGA_TYPE_RLE_COLOR_MAPPED || \
     (header).image_type == TGA_TYPE_RLE_TRUE_COLOR ||   \
     (header).image_type == TGA_TYPE_RLE_GRAYSCALE)

#define IS_COLOR_MAPPED(header)                      \
    ((header).image_type == TGA_TYPE_COLOR_MAPPED || \
     (header).image_type == TGA_TYPE_RLE_COLOR_MAPPED)

#define IS_TRUE_COLOR(header)                      \
    ((header).image_type == TGA_TYPE_TRUE_COLOR || \
     (header).image_type == TGA_TYPE_RLE_TRUE_COLOR)

#define IS_GRAYSCALE(header)                      \
    ((header).image_type == TGA_TYPE_GRAYSCALE || \
     (header).image_type == TGA_TYPE_RLE_GRAYSCALE)

#define IS_RLE(header)                                   \
    ((header).image_type == TGA_TYPE_RLE_COLOR_MAPPED || \
     (header).image_type == TGA_TYPE_RLE_TRUE_COLOR ||   \
     (header).image_type == TGA_TYPE_RLE_GRAYSCALE)

// Convert bits to integer bytes. E.g. 8 bits to 1 byte, 9 bits to 2 bytes.
#define BITS_TO_BYTES(bit_count) (((bit_count)-1) / 8 + 1)

// Checks if the picture size is correct.
// Returns false if invalid dimensisns, otherwise returns true.
bool check_dimensions(int width, int height) {
    return !(width <= 0 || width > TGA_MAX_IMAGE_DIMENSIONS || height <= 0 ||
             height > TGA_MAX_IMAGE_DIMENSIONS);
}

int pixel_format_to_pixel_size(tga::tga_pixel_format format) {
    switch (format) {
        case tga::tga_pixel_format::TGA_PIXEL_BW8:
            return 1;
        case tga::tga_pixel_format::TGA_PIXEL_BW16:
        case tga::tga_pixel_format::TGA_PIXEL_RGB555:
            return 2;
        case tga::tga_pixel_format::TGA_PIXEL_RGB24:
            return 3;
        case tga::tga_pixel_format::TGA_PIXEL_ARGB32:
            return 4;
        default:
            return -1;
    }
}

// Gets the pixel format according to the header.
// Returns true means the header is not illegal, otherwise returns false.
bool set_pixel_format(tga::tga_pixel_format &format, const tga_header &header) {
    if (IS_COLOR_MAPPED(header)) {
        // If the supported pixel_depth is changed, remember to also change
        // the pixel_to_map_index() function.
        if (header.pixel_depth == 8) {
            switch (header.map_entry_size) {
                case 15:
                case 16:
                    format = tga::tga_pixel_format::TGA_PIXEL_RGB555;
                    return true;
                case 24:
                    format = tga::tga_pixel_format::TGA_PIXEL_RGB24;
                    return true;
                case 32:
                    format = tga::tga_pixel_format::TGA_PIXEL_ARGB32;
                    return true;
            }
        }
    } else if (IS_TRUE_COLOR(header)) {
        switch (header.pixel_depth) {
            case 16:
                format = tga::tga_pixel_format::TGA_PIXEL_RGB555;
                return true;
            case 24:
                format = tga::tga_pixel_format::TGA_PIXEL_RGB24;
                return true;
            case 32:
                format = tga::tga_pixel_format::TGA_PIXEL_ARGB32;
                return true;
        }
    } else if (IS_GRAYSCALE(header)) {
        switch (header.pixel_depth) {
            case 8:
                format = tga::tga_pixel_format::TGA_PIXEL_BW8;
                return true;
            case 16:
                format = tga::tga_pixel_format::TGA_PIXEL_BW16;
                return true;
        }
    }
    return false;
}

// Used for color mapped image decode.
uint16_t pixel_to_map_index(uint8_t *pixel_ptr) {
    // Because only 8-bit index is supported now, so implemented in this way.
    return pixel_ptr[0];
}

// Gets the color of the specified index from the map.
// Returns true means no error, otherwise returns false.
bool try_get_color_from_map(uint8_t *dest, uint16_t index,
                            const color_map *map) {
    index -= map->first_index;
    if (index < 0 && index >= map->entry_count) {
        return false;
    }
    memcpy(dest, map->pixels.data() + map->bytes_per_entry * index,
           map->bytes_per_entry);
    return true;
}

// Decode image data from file stream.
// Still a C style function
tga::tga_error decode_data(uint8_t *data, const tga::tga_info *info,
                           uint8_t pixel_size, bool is_color_mapped,
                           const color_map *map, std::ifstream &stream) {
    tga::tga_error error_code = tga::tga_error::TGA_NO_ERROR;
    size_t pixel_count = (size_t)info->width * info->height;

    if (is_color_mapped) {
        for (; pixel_count > 0; --pixel_count) {
            if (stream.read((char *)data, pixel_size).gcount() != pixel_size) {
                error_code = tga::tga_error::TGA_ERROR_FILE_CANNOT_READ;
                break;
            }
            // In color mapped image, the pixel as the index value of the color
            // map. The actual pixel value is found from the color map.
            uint16_t index = pixel_to_map_index(data);
            if (!try_get_color_from_map(data, index, map)) {
                error_code = tga::tga_error::TGA_ERROR_COLOR_MAP_INDEX_FAILED;
                break;
            }
            data += map->bytes_per_entry;
        }
    } else {
        size_t data_size = pixel_count * pixel_size;
        if (stream.read((char *)data, data_size).gcount() != data_size) {
            error_code = tga::tga_error::TGA_ERROR_FILE_CANNOT_READ;
        }
    }
    return error_code;
}

// Decode image data with run-length encoding from file stream.
// Still a C style function
tga::tga_error decode_data_rle(uint8_t *data, const tga::tga_info *info,
                               uint8_t pixel_size, bool is_color_mapped,
                               const color_map *map, std::ifstream &stream) {
    tga::tga_error error_code = tga::tga_error::TGA_NO_ERROR;
    size_t pixel_count = (size_t)info->width * info->height;
    bool is_run_length_packet = false;
    uint8_t packet_count = 0;
    std::vector<uint8_t> pixel_buffer(is_color_mapped ? map->bytes_per_entry
                                                      : pixel_size);

    // The actual pixel size of the image, In order not to be confused with the
    // name of the parameter pixel_size, named data element.
    uint8_t data_element_size = pixel_format_to_pixel_size(info->pixel_format);

    for (; pixel_count > 0; --pixel_count) {
        if (packet_count == 0) {
            uint8_t repetition_count_field;
            if (stream.read((char *)&repetition_count_field, 1).gcount() != 1) {
                error_code = tga::tga_error::TGA_ERROR_FILE_CANNOT_READ;
                break;
            }
            is_run_length_packet = repetition_count_field & 0x80;
            packet_count = (repetition_count_field & 0x7F) + 1;
            if (is_run_length_packet) {
                if (stream.read((char *)pixel_buffer.data(), pixel_size)
                        .gcount() != pixel_size) {
                    error_code = tga::tga_error::TGA_ERROR_FILE_CANNOT_READ;
                    break;
                }
                if (is_color_mapped) {
                    // In color mapped image, the pixel as the index value of
                    // the color map. The actual pixel value is found from the
                    // color map.
                    uint16_t index = pixel_to_map_index(pixel_buffer.data());
                    if (!try_get_color_from_map(pixel_buffer.data(), index,
                                                map)) {
                        error_code =
                            tga::tga_error::TGA_ERROR_COLOR_MAP_INDEX_FAILED;
                        break;
                    }
                }
            }
        }

        if (is_run_length_packet) {
            memcpy(data, pixel_buffer.data(), data_element_size);
        } else {
            if (stream.read((char *)data, pixel_size).gcount() != pixel_size) {
                error_code = tga::tga_error::TGA_ERROR_FILE_CANNOT_READ;
                break;
            }
            if (is_color_mapped) {
                // Again, in color mapped image, the pixel as the index value of
                // the color map. The actual pixel value is found from the color
                // map.
                uint16_t index = pixel_to_map_index(data);
                if (!try_get_color_from_map(data, index, map)) {
                    error_code =
                        tga::tga_error::TGA_ERROR_COLOR_MAP_INDEX_FAILED;
                    break;
                }
            }
        }

        --packet_count;
        data += data_element_size;
    }

    return error_code;
}

tga::tga_error save_image(const uint8_t *data, const tga::tga_info *info,
                          std::ofstream &stream) {
    int pixel_size = pixel_format_to_pixel_size(info->pixel_format);
    uint8_t header[HEADER_SIZE];
    memset(header, 0, HEADER_SIZE);
    if (info->pixel_format == tga::tga_pixel_format::TGA_PIXEL_BW8 ||
        info->pixel_format == tga::tga_pixel_format::TGA_PIXEL_BW16) {
        header[2] = (uint8_t)TGA_TYPE_GRAYSCALE;
    } else {
        header[2] = (uint8_t)TGA_TYPE_TRUE_COLOR;
    }
    header[12] = info->width & 0xFF;
    header[13] = (info->width >> 8) & 0xFF;
    header[14] = info->height & 0xFF;
    header[15] = (info->height >> 8) & 0xFF;
    header[16] = pixel_size * 8;
    if (info->pixel_format == tga::tga_pixel_format::TGA_PIXEL_ARGB32) {
        header[17] = 0x28;
    } else {
        header[17] = 0x20;
    }

    if (!stream.write((char *)header, HEADER_SIZE)) {
        return tga::tga_error::TGA_ERROR_FILE_CANNOT_WRITE;
    }

    size_t data_size = (size_t)info->width * info->height * pixel_size;
    if (!stream.write((char *)data, data_size)) {
        return tga::tga_error::TGA_ERROR_FILE_CANNOT_WRITE;
    }

    return tga::tga_error::TGA_NO_ERROR;
}

// ----------------------tga::Image implementation----------------------
namespace tga {

Image::Image(int width, int height, tga_pixel_format format)
    : img_info{(uint16_t)width, (uint16_t)height, format} {
    if (!check_dimensions(width, height)) {
        err = tga_error::TGA_ERROR_INVALID_IMAGE_DIMENSIONS;
        return;
    }

    int pixel_size = pixel_format_to_pixel_size(format);
    if (pixel_size == -1) {
        err = tga_error::TGA_ERROR_UNSUPPORTED_PIXEL_FORMAT;
    }

    // reallocate data
    data.resize((size_t)width * height * pixel_size);

    err = tga_error::TGA_NO_ERROR;
}

Image::Image(std::string_view filepath) { load(filepath); }

bool Image::load(std::string_view filepath) {
    std::ifstream inFile(filepath.data(), std::ios::binary);
    if (!inFile.good()) {
        err = tga_error::TGA_ERROR_FILE_CANNOT_READ;
        return false;
    }

    tga_header header;

    // -----------Start load header-----------
    {
        inFile.read((char *)&header.id_length, 1);
        inFile.read((char *)&header.map_type, 1);
        inFile.read((char *)&header.image_type, 1);
        inFile.read((char *)&header.map_first_entry, 2);
        inFile.read((char *)&header.map_length, 2);
        inFile.read((char *)&header.map_entry_size, 1);
        inFile.read((char *)&header.image_x_origin, 2);
        inFile.read((char *)&header.image_y_origin, 2);
        inFile.read((char *)&header.image_width, 2);
        inFile.read((char *)&header.image_height, 2);
        inFile.read((char *)&header.pixel_depth, 1);
        inFile.read((char *)&header.image_descriptor, 1);

        if (inFile.rdstate()) {
            err = tga_error::TGA_ERROR_FILE_CANNOT_READ;
            return false;
        }
        if (header.map_type > 1) {
            err = tga_error::TGA_ERROR_UNSUPPORTED_COLOR_MAP_TYPE;
            return false;
        }
        if (header.image_type == TGA_TYPE_NO_DATA) {
            err = tga_error::TGA_ERROR_NO_DATA;
            return false;
        }
        if (!IS_SUPPORTED_IMAGE_TYPE(header)) {
            err = tga_error::TGA_ERROR_UNSUPPORTED_IMAGE_TYPE;
            return false;
        }
        if (header.image_width <= 0 || header.image_height <= 0) {
            // No need to check if the image size exceeds
            // TGA_MAX_IMAGE_DIMENSIONS.
            err = tga_error::TGA_ERROR_INVALID_IMAGE_DIMENSIONS;
            return false;
        }
        if (!set_pixel_format(img_info.pixel_format, header)) {
            err = tga_error::TGA_ERROR_UNSUPPORTED_PIXEL_FORMAT;
            return false;
        }
    }
    this->img_info.width = header.image_width;
    this->img_info.height = header.image_height;
    // img_info.pixel_format is already set

    // No need to handle the content of the ID field, so skip directly.
    if (!inFile.seekg(header.id_length, std::ios::cur)) {
        err = tga_error::TGA_ERROR_FILE_CANNOT_READ;
        return false;
    }

    bool is_color_mapped = IS_COLOR_MAPPED(header);
    bool is_rle = IS_RLE(header);

    color_map color_map;

    // -----------Handle color map field-----------
    {
        size_t map_size =
            header.map_length * BITS_TO_BYTES(header.map_entry_size);
        if (is_color_mapped) {
            color_map.first_index = header.map_first_entry;
            color_map.entry_count = header.map_length;
            color_map.bytes_per_entry = BITS_TO_BYTES(header.map_entry_size);
            color_map.pixels.resize(map_size);

            if (inFile.read((char *)color_map.pixels.data(), map_size)
                    .gcount() != map_size) {
                err = tga_error::TGA_ERROR_FILE_CANNOT_READ;
                return false;
            }
        } else if (header.map_type == 1) {
            // The image is not color mapped at this time, but contains a color
            // map. So skips the color map data block directly.
            if (!inFile.seekg(map_size, std::ios::cur)) {
                err = tga_error::TGA_ERROR_FILE_CANNOT_READ;
                return false;
            }
        }
    }

    this->data.resize((size_t)header.image_width * header.image_height *
                      pixel_format_to_pixel_size(img_info.pixel_format));

    // -----------Load image data-----------
    uint8_t pixel_size = BITS_TO_BYTES(header.pixel_depth);
    if (is_rle) {
        err = decode_data_rle(data.data(), &img_info, pixel_size,
                              is_color_mapped, &color_map, inFile);
    } else {
        err = decode_data(data.data(), &img_info, pixel_size, is_color_mapped,
                          &color_map, inFile);
    }

    if (err != tga_error::TGA_NO_ERROR) {
        return false;
    }

    // Flip the image if necessary, to keep the origin in upper left corner.
    bool b_flip_h = header.image_descriptor & 0x10;
    bool b_flip_v = !(header.image_descriptor & 0x20);
    if (b_flip_h) {
        flip_h();
    }
    if (b_flip_v) {
        flip_v();
    }

    return true;
}

bool Image::save(std::string_view filepath) {
    if (data.empty()) {
        err = tga_error::TGA_ERROR_NO_DATA;
        return false;
    }

    // Check if a file with the same name already exists.
    std::ofstream outFile(filepath.data(), std::ios::binary);
    if (!outFile.good()) {
        err = tga_error::TGA_ERROR_FILE_CANNOT_WRITE;
        return false;
    }

    err = save_image(data.data(), &img_info, outFile);
    outFile.close();  // you can't delete a file while it's opened.

    if (err != tga_error::TGA_NO_ERROR) {
        std::remove(filepath.data());
    }
    return true;
}

void Image::flip_h() {
    if (data.empty()) {
        return;
    }

    int pixel_size = pixel_format_to_pixel_size(img_info.pixel_format);
    std::vector<uint8_t> temp(pixel_size);
    int flip_num = img_info.width / 2;
    for (int i = 0; i < flip_num; ++i) {
        for (int j = 0; j < img_info.height; ++j) {
            uint8_t *p1 = get_pixel(i, j);
            uint8_t *p2 = get_pixel(img_info.width - 1 - i, j);
            // Swap two pixels.
            memcpy(temp.data(), p1, pixel_size);
            memcpy(p1, p2, pixel_size);
            memcpy(p2, temp.data(), pixel_size);
        }
    }
}

void Image::flip_v() {
    if (data.empty()) {
        return;
    }
    int pixel_size = pixel_format_to_pixel_size(img_info.pixel_format);
    std::vector<uint8_t> temp(pixel_size);
    int flip_num = img_info.height / 2;
    for (int i = 0; i < flip_num; ++i) {
        for (int j = 0; j < img_info.width; ++j) {
            uint8_t *p1 = get_pixel(j, i);
            uint8_t *p2 = get_pixel(j, img_info.height - 1 - i);
            // Swap two pixels.
            memcpy(temp.data(), p1, pixel_size);
            memcpy(p1, p2, pixel_size);
            memcpy(p2, temp.data(), pixel_size);
        }
    }
}

tga_error Image::last_error() const { return err; }

uint8_t *Image::get_pixel(int x, int y) {
    if (x < 0) {
        x = 0;
    } else if (x >= img_info.width) {
        x = img_info.width - 1;
    }
    if (y < 0) {
        y = 0;
    } else if (y >= img_info.height) {
        y = img_info.height - 1;
    }
    int pixel_size = pixel_format_to_pixel_size(img_info.pixel_format);
    return data.data() + (y * img_info.width + x) * pixel_size;
}

uint8_t *Image::get_raw_data() { return data.data(); }

std::vector<uint8_t> &Image::get_data() { return data; }

uint16_t Image::get_width() const { return img_info.width; }

uint16_t Image::get_height() const { return img_info.height; }

tga_pixel_format Image::get_pixel_format() const {
    return img_info.pixel_format;
}

uint8_t Image::get_pixel_size() const {
    return pixel_format_to_pixel_size(img_info.pixel_format);
}

const uint8_t *Image::get_raw_data() const { return data.data(); }

const std::vector<uint8_t> &Image::get_data() const { return data; }
}  // namespace tga
