# TGAFunc_Cpp

C++ port of the lightweight library written in C for handling the Truevision TGA image format.

For more information about the TGA format, please refer to the
[specification](http://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf).

## Usage

Copy the `tgafunc_cpp.h` and `tgafunc_cpp.cpp` files to your porject and include the
`tgafunc.h` in your code. Please make sure that your compiler compliant with
the C++17 standard or newer. 

> If you insist on using this header with C++11, then you'll have to manually change all the `std::string_view` to `const std::string&`

An example of how to load an existing tga image is shown below:

```c++
#include "tgafunc_cpp.h"

int main() {
    
    tga::Image img("./test/images/CBW8.tga");

    img.save("./new_file/test.tga");

    return 0;
}
```

You can use the also create an empty `tga::Image` with given width, height and format. See example below:

```c++
#include "tgafunc_cpp.h"

int main() {
    
    tga::Image default_img;     // default constructor, width = height = 0
    tga::Image img(480, 640, tga::tga_pixel_format::TGA_PIXEL_RGB24);

    // Then you can manipulate raw data with img.get_raw_data()
    uint8_t* pdata = img.get_raw_data();

    // Or you can access individual pixel with img.get_pixel(x, y)
    uint8_t* pixel_xy = img.get_pixel(114, 514);

    return 0;
}
```

## License

Licensed under the [MIT](LICENSE) license.
