//

//  main.cpp
//  opencl_xcode
//
//  Created by yang on 1/16/16.
//  Copyright © 2016 yang. All rights reserved.
//
#include "ocl/cl_wrapper.hpp"

#include <iostream>
#include <cstdlib>

std::string GetPath(std::string path)
{
#if !(defined(__APPLE__) || defined(__MACOSX))
    return path;

#else // if !(defined(__APPLE__) || defined(__MACOSX))
    return "../../../opencl_xcode/" + path;

#endif // if !(defined(__APPLE__) || defined(__MACOSX))
}

int main(int argc, const char *argv[])
{
    cl::Context context;
    cl::CommandQueue command_queue;
    cl::Kernel hello_kernel;
    bool succee_flag = false;
    std::vector<cl::Device> devices;
    cl::Program program;
    const int   data_size = 256;

    CreateContext(context);
    GetDeivces(context, devices);
    CreateCommandQueue(context, command_queue, devices.front());
    std::cout << "Initialized OpenCL success!!!" << std::endl;

    std::vector<std::string> filenames;
    filenames.push_back(GetPath("hello.cl"));
    succee_flag = CreateProgram(context,
                                devices,
                                filenames,
                                program);

    if (!succee_flag) {
        exit(-1);
    }

    hello_kernel = cl::Kernel(program, "hello");

    cl::Buffer cl_data = cl::Buffer(context,
                                    CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                    data_size * sizeof(int));

    hello_kernel.setArg(0, cl_data);
    cl::NDRange global = cl::NDRange(data_size);
    command_queue.enqueueNDRangeKernel(hello_kernel,
                                       cl::NullRange,
                                       global,
                                       cl::NullRange);

    int *pdata = (int *)command_queue.enqueueMapBuffer(cl_data,
                                                       CL_TRUE,
                                                       CL_MAP_READ,
                                                       0,
                                                       data_size * sizeof(int));

    int i = 0;

    for (i = 0; i < data_size; i++) {
        if (pdata[i] != i) {
            std::cout << "Kernel process fail!!! " << i << ";" << pdata[i] << std::endl;
            break;
        }
    }

    command_queue.enqueueUnmapMemObject(cl_data, pdata);

    if (i != data_size) {
        std::cout << "Kernel process success!!!" << std::endl;
    }

    return 0;
}
