#ifndef _OPENCL_CL_MAC_DEBUG_TOOLS_HPP_
#define _OPENCL_CL_MAC_DEBUG_TOOLS_HPP_

// // #define __CL_ENABLE_EXCEPTIONS
#if !(defined(__APPLE__) || defined(__MACOSX))
#include "CL/cl.hpp"
#else
#include "cl.hpp"
#endif

#include <sys/time.h>
#include <stdio.h>

#define PROFILE
#ifdef PROFILE
# define PROFILE_INIT(name, enable)                               \
    static struct timeval in_timeval_profile_ ## name ## _point,  \
                          out_timeval_profile_ ## name ## _point; \
    static long sum_time_profile_ ## name ## _point = 0;          \
    static long total_processed_frames_ ## name ## _point = 0;    \
    static bool enable_profile_ ## name = enable

# define PROFILE_IN(name)                                           \
    do {                                                            \
        gettimeofday(&in_timeval_profile_ ## name ## _point, NULL); \
    } while (0)

# define PROFILE_OUT_SUM(name, sum)                                                                \
    do {                                                                                           \
        gettimeofday(&out_timeval_profile_ ## name ## _point, NULL);                               \
        sum_time_profile_ ## name ## _point += (out_timeval_profile_ ## name ## _point.tv_usec -   \
                                                in_timeval_profile_ ## name ## _point.tv_usec) +   \
                                               1000000 *                                           \
                                               (out_timeval_profile_ ## name ## _point.tv_sec -    \
                                                in_timeval_profile_ ## name ## _point.             \
                                                tv_sec);                                           \
        total_processed_frames_ ## name ## _point++;                                               \
        if (enable_profile_ ## name == true && sum >= total_processed_frames_ ## name ## _point) { \
            printf(# name " -- profile point %ld us\n",                                            \
                   sum_time_profile_ ## name ## _point);                                           \
            sum_time_profile_ ## name ## _point       = 0;                                         \
            total_processed_frames_ ## name ## _point = 0;                                         \
        }                                                                                          \
    } while (0)

# define PROFILE_OUT(name) PROFILE_OUT_SUM(name, 1)
#else // ifdef PROFILE
# define PROFILE_INIT(name, enable)
# define PROFILE_IN(name)
# define PROFILE_OUT(name)
# define PROFILE_OUT_SUM(name, sum)
#endif // ifdef PROFILE

#if (defined(__APPLE__) || defined(__MACOSX))
int ShowU8RGBImageCPU(unsigned char *image,
                      int            width,
                      int            height);

int ShowU8ImageGPUBuffer(cl::CommandQueue command_queue,
                         cl::Buffer       buffer,
                         int              width,
                         int              height);

int ShowS16ImageGPUBuffer(cl::CommandQueue command_queue,
                          cl::Buffer       buffer,
                          int              width,
                          int              height);

int ShowU8GrayImageCPU(unsigned char *image, int width, int height);
#endif // if !(defined(__APPLE__) || defined(__MACOSX))

#endif // ifndef _OPENCL_CL_MAC_DEBUG_TOOLS_HPP_
