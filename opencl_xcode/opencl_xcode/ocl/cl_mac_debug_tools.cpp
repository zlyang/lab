#include "cl_mac_debug_tools.hpp"

#include <opencv2/opencv.hpp>

#include "cl_common.hpp"
#include "cl_wrapper.hpp"

using namespace cv;

int ResizeImage(Mat& img_to_show, int width, int height)
{
    const int max_width = 720;
    int resize_width, resize_height;

    resize_width  = width > max_width ? max_width : width;
    resize_height = (int)((float)resize_width / width * height);

    resize(img_to_show, img_to_show, Size(resize_width, resize_height));

    return 0;
}

int ShowU8GrayImageCPU(unsigned char *image, int width, int height)
{
    Mat img_to_show(height, width, CV_8UC1, image);

    ResizeImage(img_to_show, width, height);
    imshow("gray", img_to_show);
    waitKey();

    return (0);
}

int ShowU8RGBImageCPU(unsigned char *image, int width, int height)
{
    Mat img_to_show(height, width, CV_8UC3, image);

    ResizeImage(img_to_show, width, height);
    cvtColor(img_to_show, img_to_show, CV_RGB2BGR);
    imshow("rgb", img_to_show);
    waitKey();

    return (0);
}

int ShowU8ImageGPUBuffer(cl::CommandQueue command_queue,
                         cl::Buffer       buffer,
                         int              width,
                         int              height)
{
    unsigned char *cpu = (unsigned char *)command_queue.enqueueMapBuffer(buffer,
                                                                         CL_TRUE,
                                                                         CL_MAP_WRITE,
                                                                         0,
                                                                         width * height);
    Mat img_to_show(height, width, CV_8U, cpu);

    ResizeImage(img_to_show, width, height);
    imshow("u8", img_to_show);
    waitKey();

    command_queue.enqueueUnmapMemObject(buffer, cpu);

    return (0);
}

int ShowS16ImageGPUBuffer(cl::CommandQueue command_queue,
                          cl::Buffer       buffer,
                          int              width,
                          int              height)
{
    unsigned char *cpu = (unsigned char *)command_queue.enqueueMapBuffer(buffer,
                                                                         CL_TRUE,
                                                                         CL_MAP_WRITE,
                                                                         0,
                                                                         sizeof(short) * width *
                                                                         height);
    Mat img_to_show(height, width, CV_16SC1, cpu);

    ResizeImage(img_to_show, width, height);
    imshow("s16", img_to_show);
    waitKey();

    command_queue.enqueueUnmapMemObject(buffer, cpu);

    return (0);
}
