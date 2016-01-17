#ifndef _OPENCL_CL_WRAPPER_H_
#define _OPENCL_CL_WRAPPER_H_

#if !(defined(__APPLE__) || defined(__MACOSX))
#include "CL/cl.hpp"
#else
#include "cl.hpp"
#endif
#include "cl_common.hpp"

#include "cl_wrapper.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

/**
 * [print event profile]
 * @param  info  [distinguish for show]
 * @param  event [profile event]
 * @return       [true for success]
 */
bool PrintProfilingInfo(std::string info,
                        cl::Event   event);

/**
 * [create opencl context]
 * @param  context [the new context]
 * @return         [true for success]
 */
bool CreateContext(cl::Context& context);

/**
 * [get opencl device ids]
 * @param  context [opencl context]
 * @param  devices [return devices list]
 * @return         [true for success]
 */
bool GetDeivces(cl::Context              context,
                std::vector<cl::Device>& devices);

/**
 * [create command queue for device]
 * @param  context       [opencl context]
 * @param  command_queue [return comand queue]
 * @param  device        [command queue bind to]
 * @return               [true if success]
 */
bool CreateCommandQueue(cl::Context       context,
                        cl::CommandQueue& command_queue,
                        cl::Device        device);

/**
 * [create program from file]
 * @param  context  [opencl context]
 * @param  devices  [opencl device]
 * @param  filename [program file name]
 * @param  program  [return program]
 * @return          [true if success]
 */
bool CreateProgram(cl::Context              context,
                   std::vector<cl::Device>& devices,
                   std::vector<std::string>filenames,
                   cl::Program            & program);

/**
 * [print error string & convert error number to string]
 * @param  error_number [error number]
 * @return              [true if success]
 */
inline bool CheckSuccess(cl_int error_number);

/**
 * error number to string
 * @param  error_number [error number]
 * @return              [true if success]
 */
std::string ErrorNumberToString(cl_int error_number);

/**
 * check if the device support the give extension
 * @param  device    [opencl device]
 * @param  extension [check extension string]
 * @return           [true if success]
 */
bool        IsExtensionSupported(cl::Device  device,
                                 std::string extension);

/**
 * print the context support image format
 * @param  context [opencl context]
 * @param  flag    [memory flags;e.g. CL_MEM_READ_WRITE]
 * @param  type    [image type;e.g CL_MEM_OBJECT_IMAGE2D]
 * @return         [true if success]
 */
bool PrintSupportedImageFormats(cl::Context        context,
                                cl_mem_flags       flag,
                                cl_mem_object_type type);

#endif // ifndef _OPENCL_CL_WRAPPER_H_
