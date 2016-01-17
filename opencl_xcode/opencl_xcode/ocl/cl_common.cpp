#include "cl_common.hpp"
#include <fstream>
#include <iostream>

using namespace std;

struct BitmapMagic {
    unsigned char magic[2];
};

/**
 * \brief Bitmap header.
 * \details See the BMP file format specification for more details.
 */
struct BitmapHeader {
    uint32_t file_size; /**< \brief Total size of the bitmap in bytes. */
    uint16_t creator1;  /**< \brief Reserved field which can be application
                           defined. */
    uint16_t creator2;  /**< \brief Reserved field which can be application
                           defined. */
    uint32_t offset;    /**< \brief Offset in bytes to the beginning of the
                           image
                           data block. */
};

/**
 * \brief Bitmap information header.
 * \details See the BMP file format specification for more details.
 */
struct BitmapInformationHeader {
    uint32_t size;                    /**< Size of the information headers in
                                         bytes. */
    int32_t  width;                   /**< Width of the image. */
    int32_t  height;                  /**< Height of the image. */
    uint16_t number_color_planes;     /**< The number of colour planes. The only
                                         legal value is 1. */
    uint16_t bits_per_pixel;          /**< Number of bits per pixel in the
                                         image. */
    uint32_t compression_type;        /**< Compression type. Use 0 for
                                         uncompressed. */
    uint32_t raw_bitmap_size;         /**< Size of the image data including
                                         padding (does not include the size of
                                         the headers). */
    int32_t  horizontal_resolution;   /**< Resolution is in pixels per meter. */
    int32_t  vertical_resolution;     /**< Resolution is in pixels per meter. */
    uint32_t number_colors;           /**< Number of colours in the image, can
                                          be left as 0. */
    uint32_t number_important_colors; /**< Generally ignored by applications. */
};

bool SaveToBitmap(string filename, int width, int height, const unsigned char *image_data)
{
    /* Try and open the file for writing. */
    fstream image_file(filename.c_str(), ios::out);

    if (!image_file.is_open()) {
        cerr << "Unable to open " << filename << ". " << __FILE__ << ":" << __LINE__ <<
            endl;
        return false;
    }

    /* Magic header bits come from the bitmap specification. */
    const struct BitmapMagic magic = { { 0x42, 0x4d } };
    struct BitmapHeader header;
    struct BitmapInformationHeader information_header;

    /*
     * Each row of the data must be padded to a multiple of 4 bytes according to
     * the bitmap specification.
     * This method uses three bytes per pixel (hence the width * 3).
     * Then we increase the padding until it is divisible by 4.
     */
    int padded_width = width * 3;

    while ((padded_width % 4) != 0) {
        padded_width++;
    }

    /* Setup the bitmap header. */
    header.file_size = sizeof(magic) + sizeof(header) + sizeof(information_header) +
                       padded_width * height;
    header.creator1 = 0;
    header.creator2 = 0;
    header.offset   = sizeof(magic) + sizeof(header) + sizeof(information_header);

    /* Setup the bitmap information header. */
    information_header.size                    = sizeof(information_header);
    information_header.width                   = width;
    information_header.height                  = height;
    information_header.number_color_planes     = 1;
    information_header.bits_per_pixel          = 24;
    information_header.compression_type        = 0;
    information_header.raw_bitmap_size         = padded_width * height;
    information_header.horizontal_resolution   = 2835;
    information_header.vertical_resolution     = 2835;
    information_header.number_colors           = 0;
    information_header.number_important_colors = 0;

    /* Try to write the header data. */
    if (image_file.write((char *)&magic, sizeof(magic)).bad() ||
        image_file.write((char *)&header, sizeof(header)).bad() ||
        image_file.write((char *)&information_header, sizeof(information_header)).bad()) {
        cerr << "Failed to write bitmap header. " << __FILE__ << ":" << __LINE__ << endl;

        if (image_file.is_open()) {
            image_file.close();
        }
        return false;
    }

    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            /* The pixels lie in RGB order in memory, we need to store them in
               BGR order. */
            unsigned char rgb[3];
            rgb[2] = image_data[3 * (y * information_header.width + x) + 0];
            rgb[1] = image_data[3 * (y * information_header.width + x) + 1];
            rgb[0] = image_data[3 * (y * information_header.width + x) + 2];

            if (image_file.write((char *)&rgb, 3).bad()) {
                if (image_file.is_open()) {
                    image_file.close();
                }
                return false;
            }
        }

        /*
         * At the end of the row, write out blank bytes to ensure the row length
         * is
         * a multiple of 4 bytes (part of the bitmap specification).
         */
        for (int x = width * 3; x < padded_width; x++) {
            char b = 0;

            if (image_file.write(&b, 1).bad()) {
                if (image_file.is_open()) {
                    image_file.close();
                }
                return false;
            }
        }
    }

    return true;
} // SaveToBitmap

bool LoadFromBitmap(const string    filename,
                    int *const      width,
                    int *const      height,
                    unsigned char **image_data)
{
    /* Try and open the file for reading. */
    ifstream image_file(filename.c_str(), ios::in);

    if (!image_file.is_open()) {
        cerr << "Unable to open " << filename << ". " << __FILE__ << ":" << __LINE__ <<
            endl;
        return false;
    }

    /*
     * Read and check the headers to make sure we support the type of bitmap
     * passed in.
     */
    struct BitmapMagic  magic;
    struct BitmapHeader header;
    struct BitmapInformationHeader information_header;

    if (image_file.read((char *)&magic,
                        sizeof(magic)).bad() || (magic.magic[0] != 0x42) ||
        (magic.magic[1] != 0x4d)) {
        /* Not a valid BMP file header */
        cerr << "Not a valid BMP file header. " << __FILE__ << ":" << __LINE__ << endl;

        if (image_file.is_open()) {
            image_file.close();
        }
        return false;
    }

    /* 54 is the standard size of a bitmap header. */
    if (image_file.read((char *)&header, sizeof(header)).bad() || (header.offset != 54)) {
        /* Not a supported BMP format */
        cerr << "Not a supported BMP format. " << __FILE__ << ":" << __LINE__ << endl;

        if (image_file.is_open()) {
            image_file.close();
        }
        return false;
    }

    if (image_file.read((char *)&information_header,
                        sizeof(information_header)).bad() ||
        (information_header.compression_type != 0) ||
        (information_header.bits_per_pixel != 24)) {
        /* We only support uncompressed 24-bits per pixel RGB */
        cerr << "We only support uncompressed 24-bits per pixel RGB. " << __FILE__ << ":" <<
            __LINE__ << endl;

        if (image_file.is_open()) {
            image_file.close();
        }
        return false;
    }

    int row_delta;
    int first_row;
    int after_last_row;

    if (information_header.height > 0) {
        /* The image is stored upside down in memory */
        row_delta      = -1;
        first_row      = information_header.height - 1;
        after_last_row = -1;
    } else {
        information_header.height = -information_header.height;
        row_delta                 = 1;
        first_row                 = 0;
        after_last_row            = information_header.height;
    }

    /* Calculate the paddle of the image to skip it when reading the buffer. */
    int padded_width = information_header.width * 3;

    while ((padded_width % 4) != 0) {
        padded_width++;
    }

    /* 24-bits per pixel means 3 bytes of data per pixel. */
    int size = 3 * padded_width * information_header.height;
    *image_data = new unsigned char[size];
    unsigned char *read_buffer = new unsigned char[size];

    /* Try to read in the image data. */
    if (image_file.read((char *)read_buffer, size).bad()) {
        cerr << "Error reading main image data. " << __FILE__ << ":" << __LINE__ << endl;

        if (image_file.is_open()) {
            image_file.close();
        }

        if (read_buffer != NULL) {
            delete[] read_buffer;
        }
        return false;
    }

    int read_buffer_index = 0;

    /* Loop throught the image data and store it at the output data location. */
    for (int y = first_row; y != after_last_row; y += row_delta) {
        for (int x = 0; x < information_header.width; x++) {
            /* The pixels lie in BGR order, we need to resort them into RGB */
            (*image_data)[3 * (y * information_header.width + x) +
                          0] = read_buffer[read_buffer_index + 2];
            (*image_data)[3 * (y * information_header.width + x) +
                          1] = read_buffer[read_buffer_index + 1];
            (*image_data)[3 * (y * information_header.width + x) +
                          2] = read_buffer[read_buffer_index + 0];

            read_buffer_index += 3;
        }

        /* Skip padding. */
        read_buffer_index += padded_width - (information_header.width * 3);
    }

    *width  = information_header.width;
    *height = information_header.height;

    if (image_file.is_open()) {
        image_file.close();
    }

    if (read_buffer != NULL) {
        delete[] read_buffer;
    }

    return true;
} // LoadFromBitmap

bool LuminanceToRGB(const unsigned char *luminance_data,
                    unsigned char       *rgb_data,
                    int                  width,
                    int                  height)
{
    RETURE_FLASE_IF_NULL(rgb_data,       "rgb_data cannot be NULL. ");
    RETURE_FLASE_IF_NULL(luminance_data, "luminance_data cannot be NULL. ");

    for (int n = width * height - 1; n >= 0; --n) {
        unsigned char d = luminance_data[n];
        rgb_data[3 * n + 0] = d;
        rgb_data[3 * n + 1] = d;
        rgb_data[3 * n + 2] = d;
    }
    return true;
}

bool RGBToLuminance(const unsigned char *const rgb_data,
                    unsigned char *const       luminance_data,
                    int                        width,
                    int                        height)
{
    RETURE_FLASE_IF_NULL(rgb_data,       "rgb_data cannot be NULL. ");
    RETURE_FLASE_IF_NULL(luminance_data, "luminance_data cannot be NULL. ");

    for (int n = width * height - 1; n >= 0; --n) {
        float r = rgb_data[3 * n + 0];
        float g = rgb_data[3 * n + 1];
        float b = rgb_data[3 * n + 2];
        luminance_data[n] = (unsigned char)(0.2126f * r + 0.7152f * g + 0.0722f * b + 0.5);
    }
    return true;
}

bool GrayToRGB(const unsigned char *gray_data,
               unsigned char       *rgb_data,
               int                  width,
               int                  height)
{
    RETURE_FLASE_IF_NULL(rgb_data,  "rgb_data cannot be NULL. ");
    RETURE_FLASE_IF_NULL(gray_data, "gray_data cannot be NULL. ");

    for (int n = width * height - 1; n >= 0; --n) {
        unsigned char d = gray_data[n];
        rgb_data[3 * n + 0] = d;
        rgb_data[3 * n + 1] = d;
        rgb_data[3 * n + 2] = d;
    }
    return true;
}

bool RGBToGray(const unsigned char *const rgb_data,
               unsigned char *const       gray_data,
               int                        width,
               int                        height)
{
    RETURE_FLASE_IF_NULL(rgb_data,  "rgb_data cannot be NULL. ");
    RETURE_FLASE_IF_NULL(gray_data, "gray_data cannot be NULL. ");

    for (int n = width * height - 1; n >= 0; --n) {
        float r = rgb_data[3 * n + 0];
        float g = rgb_data[3 * n + 1];
        float b = rgb_data[3 * n + 2];
        gray_data[n] = (unsigned char)(0.299f * r + 0.587f * g + 0.114f * b + 0.5);
    }
    return true;
}

bool RGBToRGBA(const unsigned char *const rgb_data,
               unsigned char *const       rgba_data,
               int                        width,
               int                        height)
{
    RETURE_FLASE_IF_NULL(rgb_data,  "rgb_data cannot be NULL. ");
    RETURE_FLASE_IF_NULL(rgba_data, "rgba_data cannot be NULL. ");

    for (int n = 0; n < width * height; n++) {
        /* Copy the RGB components directly. */
        rgba_data[4 * n + 0] = rgb_data[3 * n + 0];
        rgba_data[4 * n + 1] = rgb_data[3 * n + 1];
        rgba_data[4 * n + 2] = rgb_data[3 * n + 2];

        /* Set the alpha channel to 255 (fully opaque). */
        rgba_data[4 * n + 3] = (unsigned char)255;
    }

    return true;
}

bool RGBAToRGB(const unsigned char *const __restrict rgba_data,
               unsigned char *const __restrict       rgb_data,
               int                                   width,
               int                                   height)
{
    RETURE_FLASE_IF_NULL(rgba_data, "rgba_data cannot be NULL. ");
    RETURE_FLASE_IF_NULL(rgb_data,  "rgb_data cannot be NULL. ");

    for (int n = 0; n < width * height; n++) {
        /* Copy the RGB components but throw away the alpha channel. */
        rgb_data[3 * n + 0] = rgba_data[4 * n + 0];
        rgb_data[3 * n + 1] = rgba_data[4 * n + 1];
        rgb_data[3 * n + 2] = rgba_data[4 * n + 2];
    }

    return true;
}

bool SaveToFile(string filename, int size, unsigned char *image_data)
{
    fstream file(filename.c_str(), ios::out);

    if (!file.is_open()) {
        cerr << "Unable to open " << filename << ". " << __FILE__ << ":" << __LINE__ <<
            endl;
        return false;
    }

    for (int x = 0; x < size; x++) {
        if (file.write((char *)&image_data[x], 1).bad()) {
            if (file.is_open()) {
                file.close();
            }
            return false;
        }
    }

    return true;
}
