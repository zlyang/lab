#include "ion_wrapper.hpp"
#include "ocl/cl_common.hpp"
#include "ocl/cl_wrapper.hpp"
#include "ocl/cl_mac_debug_tools.hpp"

#include <CL/cl_ext_qcom.h>
#include <iostream>
#include <cstdlib>

PROFILE_INIT(map_time_malloc, true);
PROFILE_INIT(map_time_ion, true);
int main(int argc, const char *argv[])
{
    int rv = 0;
    cl::Context context;
    cl::CommandQueue command_queue;
    bool succee_flag = true;
    std::vector<cl::Device> devices;
    cl::Program program;

    const int buffer_size = 1024 * 1024 * 128;

    IonBuffer ion_ptr;
    cl_mem_ion_host_ptr cl_ion_ptr;

    CreateContext(context);
    GetDeivces(context, devices);
    CreateCommandQueue(context, command_queue, devices.front());

    std::vector<std::string> filenames;
    filenames.push_back("cl/hello.cl");
    succee_flag = CreateProgram(context,
                                devices,
                                filenames,
                                program);

    cl::Kernel hello_kernel = cl::Kernel(program, "hello");

    if (!succee_flag) {
        exit(-1);
    }

    rv = ion_allocate(buffer_size, ion_ptr);

    if (rv < 0) {
        exit(-2);
    }

    cl_ion_ptr.ext_host_ptr.allocation_type = CL_MEM_ION_HOST_PTR_QCOM;
    cl_ion_ptr.ext_host_ptr.host_cache_policy = CL_MEM_HOST_UNCACHED_QCOM;
    cl_ion_ptr.ion_filedesc = ion_ptr.fd_data.fd;
    cl_ion_ptr.ion_hostptr = ion_ptr.vaddr;

    cl::Buffer buffer_ion = cl::Buffer(context,
                                       CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR | CL_MEM_EXT_HOST_PTR_QCOM,
                                       buffer_size,
                                       &cl_ion_ptr);

    hello_kernel.setArg(0, buffer_ion);
    cl::NDRange global = cl::NDRange(buffer_size / sizeof(int));
    command_queue.enqueueNDRangeKernel(hello_kernel,
                                       cl::NullRange,
                                       global,
                                       cl::NullRange, NULL, NULL);
    command_queue.finish();

    PROFILE_IN(map_time_ion);
    int *pclresult = (int *)command_queue.enqueueMapBuffer(buffer_ion,
                                                           CL_TRUE,
                                                           CL_MAP_READ,
                                                           0,
                                                           buffer_size);

    for (int i = 0; i < buffer_size / sizeof(int); i++) {
        if (i != pclresult[i]) {
            std::cout << "diff: " << i << ";" << pclresult[i] << std::endl;
            break;
        }
    }

    command_queue.enqueueUnmapMemObject(buffer_ion, pclresult);
    command_queue.finish();

    PROFILE_OUT(map_time_ion);   // 333ms

    cl::Buffer buffer_malloc = cl::Buffer(context,
                                          CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                                          buffer_size);

    hello_kernel.setArg(0, buffer_malloc);
    command_queue.enqueueNDRangeKernel(hello_kernel,
                                       cl::NullRange,
                                       global,
                                       cl::NullRange, NULL, NULL);
    command_queue.finish();

    PROFILE_IN(map_time_malloc);
    pclresult = (int *)command_queue.enqueueMapBuffer(buffer_malloc,
                                                      CL_TRUE,
                                                      CL_MAP_READ,
                                                      0,
                                                      buffer_size);

    for (int i = 0; i < buffer_size / sizeof(int); i++) {
        if (i != pclresult[i]) {
            std::cout << "diff: " << i << ";" << pclresult[i] << std::endl;
            break;
        }
    }

    command_queue.enqueueUnmapMemObject(buffer_malloc, pclresult);

    PROFILE_OUT(map_time_malloc);   // 12.8s   40x
    command_queue.finish();

    return 0;
} // main
