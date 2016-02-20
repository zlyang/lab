#ifndef _OPENCL_CL_COMMON_HPP_
#define _OPENCL_CL_COMMON_HPP_

#include <string>

#define RETURE_FLASE_IF_NULL(para, info)     \
    if (para == NULL) {                      \
        std::cerr << info << __FILE__ << ":" \
                  << __LINE__ << std::endl;  \
        return false;                        \
    }

#define CL_WARN(info) \
    std::cout << "warn:" << info << __FILE__ << ":" << __LINE__ << std::endl

bool SaveToBitmap(std::string          filename,
                  int                  width,
                  int                  height,
                  const unsigned char *imageData);

bool LoadFromBitmap(std::string     filename,
                    int            *width,
                    int            *height,
                    unsigned char **imageData);

bool LuminanceToRGB(const unsigned char *luminanceData,
                    unsigned char       *rgbData,
                    int                  width,
                    int                  height);

bool RGBToLuminance(const unsigned char *rgbData,
                    unsigned char       *luminanceData,
                    int                  width,
                    int                  height);

bool RGBToRGBA(const unsigned char *rgbData,
               unsigned char       *rgbaData,
               int                  width,
               int                  height);

bool RGBAToRGB(const unsigned char *rgbaData,
               unsigned char       *rgbData,
               int                  width,
               int                  height);

bool RGBAToRGB(const unsigned char *rgbaData,
               unsigned char       *rgbData,
               int                  width,
               int                  height);

bool SaveToFile(std::string    filename,
                int            size,
                unsigned char *image_data);

#endif // ifndef _OPENCL_CL_COMMON_HPP_
