#include "cl_wrapper.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;
using namespace cl;

bool PrintProfilingInfo(std::string info, Event event)
{
    cl_ulong queued_time = 0;

    if (!CheckSuccess(event.getProfilingInfo(CL_PROFILING_COMMAND_QUEUED, &queued_time))) {
        CL_WARN("Retrieving CL_PROFILING_COMMAND_QUEUED OpenCL profiling information failed. ");
        return (false);
    }

    cl_ulong submitted_time = 0;

    if (!CheckSuccess(event.getProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, &submitted_time))) {
        CL_WARN("Retrieving CL_PROFILING_COMMAND_SUBMIT OpenCL profiling information failed. ");
        return (false);
    }

    cl_ulong start_time = 0;

    if (!CheckSuccess(event.getProfilingInfo(CL_PROFILING_COMMAND_START, &start_time))) {
        CL_WARN("Retrieving CL_PROFILING_COMMAND_START OpenCL profiling information failed. ");
        return (false);
    }

    cl_ulong end_time = 0;

    if (!CheckSuccess(event.getProfilingInfo(CL_PROFILING_COMMAND_END, &end_time))) {
        CL_WARN("Retrieving CL_PROFILING_COMMAND_END OpenCL profiling information failed. ");
        return (false);
    }

    cout << info << "  --  Profiling information:\n";

    /* OpenCL returns times in nano seconds. Print out the times in milliseconds
       (divide by a million). */
    cout << "Queued time: \t" << (submitted_time - queued_time) / 1000000.0 << "ms\n";
    cout << "Wait time: \t" << (start_time - submitted_time) / 1000000.0 << "ms\n";
    cout << "Run time: \t" << (end_time - start_time) / 1000000.0 << "ms" << endl;

    return (true);
}

bool CreateContext(Context& context)
{
    cl_int error_number = 0;

    std::vector<Platform> platforms;

    /* Retrieve a single platform ID. */
    error_number = Platform::get(&platforms);

    if (error_number < 0) {
        CL_WARN("No OpenCL platforms found. ");
        return (false);
    }

    cout << "platforms count :" << platforms.size() << endl;

    /* Get a context with a GPU device from the platform found above. */

    // cl_context_properties context_properties[] =
    // { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms.front(), 0 };
    context = Context((cl_device_type)CL_DEVICE_TYPE_GPU, NULL, NULL, NULL, &error_number);

    return (error_number);
}

bool GetDeivces(Context context, std::vector<Device>& devices)
{
    cl_int error_number;

    devices = context.getInfo<CL_CONTEXT_DEVICES>(&error_number);

    if (error_number < 0) {
        CL_WARN("get devices info fail");
        return (false);
    }

    return (true);
}

bool CreateCommandQueue(Context context, CommandQueue& command_queue, Device device)
{
    cl_int error_number = 0;

    /* Set up the command queue with the selected device. */
    command_queue = CommandQueue(context,
                                 device,
                                 (cl_command_queue_properties)CL_QUEUE_PROFILING_ENABLE,
                                 &error_number);

    if (error_number < 0) {
        CL_WARN("Failed to create the OpenCL command queue. ");
        return (false);
    }

    return (true);
}

bool CreateProgram(Context                 context,
                   std::vector<Device>   & devices,
                   std::vector<std::string>filenames,
                   Program               & program)
{
    cl_int error_number = 0;
    Program::Sources source;

    for (int i = 0; i < filenames.size(); i++) {
        ifstream kernel_file(filenames[i].c_str(), ios::in);

        if (!kernel_file.is_open()) {
            CL_WARN("Unable to open " + filenames[i]);
            return (false);
        }

        /*
         * Read the kernel file into an output stream.
         * Convert this into a char array for passing to OpenCL.
         */
        ostringstream output_string_stream;
        output_string_stream << kernel_file.rdbuf();
        std::string *src_std_str = new std::string(output_string_stream.str());
        const char *char_source = (*src_std_str).c_str();

        source.push_back(std::make_pair(char_source, strlen(char_source)));
    }

    program = Program(context, source, &error_number);

    if (error_number < 0) {
        cerr << "Failed to create OpenCL program. " << __FILE__ << ":" << __LINE__ << endl;
        return (false);
    }

    /* Try to build the OpenCL program. */

    // char   build_para[128] = "-cl-opt-disable";
    cl_int build_success = program.build(devices);

    /*
     * If the build succeeds with no log, an empty string is returned (logSize =
     * 1),
     * we only want to print the message if it has some content (logSize > 1).
     */
    std::string log;
    error_number = program.getBuildInfo(devices.front(), CL_PROGRAM_BUILD_LOG, &log);

    if ((error_number <= 0) && (build_success < 0)) {
        cerr << "Build log:\n " << log << endl;
    }

    if (build_success < 0) {
        CL_WARN("Failed to build OpenCL program. ");
        return (false);
    }

    return (true);
}

inline bool CheckSuccess(cl_int error_number)
{
    if (error_number != CL_SUCCESS) {
        cerr << "OpenCL error: " << ErrorNumberToString(error_number) << endl;
        return (false);
    }
    return (true);
}

std::string ErrorNumberToString(cl_int error_number)
{
    switch (error_number) {
    case CL_SUCCESS:
        return ("CL_SUCCESS");

    case CL_DEVICE_NOT_FOUND:
        return ("CL_DEVICE_NOT_FOUND");

    case CL_DEVICE_NOT_AVAILABLE:
        return ("CL_DEVICE_NOT_AVAILABLE");

    case CL_COMPILER_NOT_AVAILABLE:
        return ("CL_COMPILER_NOT_AVAILABLE");

    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        return ("CL_MEM_OBJECT_ALLOCATION_FAILURE");

    case CL_OUT_OF_RESOURCES:
        return ("CL_OUT_OF_RESOURCES");

    case CL_OUT_OF_HOST_MEMORY:
        return ("CL_OUT_OF_HOST_MEMORY");

    case CL_PROFILING_INFO_NOT_AVAILABLE:
        return ("CL_PROFILING_INFO_NOT_AVAILABLE");

    case CL_MEM_COPY_OVERLAP:
        return ("CL_MEM_COPY_OVERLAP");

    case CL_IMAGE_FORMAT_MISMATCH:
        return ("CL_IMAGE_FORMAT_MISMATCH");

    case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        return ("CL_IMAGE_FORMAT_NOT_SUPPORTED");

    case CL_BUILD_PROGRAM_FAILURE:
        return ("CL_BUILD_PROGRAM_FAILURE");

    case CL_MAP_FAILURE:
        return ("CL_MAP_FAILURE");

    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
        return ("CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST");

    case CL_INVALID_VALUE:
        return ("CL_INVALID_VALUE");

    case CL_INVALID_DEVICE_TYPE:
        return ("CL_INVALID_DEVICE_TYPE");

    case CL_INVALID_PLATFORM:
        return ("CL_INVALID_PLATFORM");

    case CL_INVALID_DEVICE:
        return ("CL_INVALID_DEVICE");

    case CL_INVALID_CONTEXT:
        return ("CL_INVALID_CONTEXT");

    case CL_INVALID_QUEUE_PROPERTIES:
        return ("CL_INVALID_QUEUE_PROPERTIES");

    case CL_INVALID_COMMAND_QUEUE:
        return ("CL_INVALID_COMMAND_QUEUE");

    case CL_INVALID_HOST_PTR:
        return ("CL_INVALID_HOST_PTR");

    case CL_INVALID_MEM_OBJECT:
        return ("CL_INVALID_MEM_OBJECT");

    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        return ("CL_INVALID_IMAGE_FORMAT_DESCRIPTOR");

    case CL_INVALID_IMAGE_SIZE:
        return ("CL_INVALID_IMAGE_SIZE");

    case CL_INVALID_SAMPLER:
        return ("CL_INVALID_SAMPLER");

    case CL_INVALID_BINARY:
        return ("CL_INVALID_BINARY");

    case CL_INVALID_BUILD_OPTIONS:
        return ("CL_INVALID_BUILD_OPTIONS");

    case CL_INVALID_PROGRAM:
        return ("CL_INVALID_PROGRAM");

    case CL_INVALID_PROGRAM_EXECUTABLE:
        return ("CL_INVALID_PROGRAM_EXECUTABLE");

    case CL_INVALID_KERNEL_NAME:
        return ("CL_INVALID_KERNEL_NAME");

    case CL_INVALID_KERNEL_DEFINITION:
        return ("CL_INVALID_KERNEL_DEFINITION");

    case CL_INVALID_KERNEL:
        return ("CL_INVALID_KERNEL");

    case CL_INVALID_ARG_INDEX:
        return ("CL_INVALID_ARG_INDEX");

    case CL_INVALID_ARG_VALUE:
        return ("CL_INVALID_ARG_VALUE");

    case CL_INVALID_ARG_SIZE:
        return ("CL_INVALID_ARG_SIZE");

    case CL_INVALID_KERNEL_ARGS:
        return ("CL_INVALID_KERNEL_ARGS");

    case CL_INVALID_WORK_DIMENSION:
        return ("CL_INVALID_WORK_DIMENSION");

    case CL_INVALID_WORK_GROUP_SIZE:
        return ("CL_INVALID_WORK_GROUP_SIZE");

    case CL_INVALID_WORK_ITEM_SIZE:
        return ("CL_INVALID_WORK_ITEM_SIZE");

    case CL_INVALID_GLOBAL_OFFSET:
        return ("CL_INVALID_GLOBAL_OFFSET");

    case CL_INVALID_EVENT_WAIT_LIST:
        return ("CL_INVALID_EVENT_WAIT_LIST");

    case CL_INVALID_EVENT:
        return ("CL_INVALID_EVENT");

    case CL_INVALID_OPERATION:
        return ("CL_INVALID_OPERATION");

    case CL_INVALID_GL_OBJECT:
        return ("CL_INVALID_GL_OBJECT");

    case CL_INVALID_BUFFER_SIZE:
        return ("CL_INVALID_BUFFER_SIZE");

    case CL_INVALID_MIP_LEVEL:
        return ("CL_INVALID_MIP_LEVEL");

    default:
        return ("Unknown error");
    }
}

bool IsExtensionSupported(Device device, std::string extension)
{
    if (extension.empty()) {
        return (false);
    }

    /* See if the requested extension is in the list. */
    std::string extensions_string;
    cl_int error_number = device.getInfo(CL_DEVICE_EXTENSIONS, &extensions_string);

    if (error_number < 0) {
        CL_WARN("Failed to get data from clGetDeviceInfo for parameter CL_DEVICE_EXTENSIONS");
        return (false);
    }

    bool return_result = false;

    if (extensions_string.find(extension) != std::string::npos) {
        return_result = true;
    }

    return (return_result);
}

std::string imageChannelOrderToString(cl_channel_order channelOrder)
{
    switch (channelOrder) {
    case CL_R:
        return ("CL_R");

    case CL_A:
        return ("CL_A");

    case CL_RG:
        return ("CL_RG");

    case CL_RA:
        return ("CL_RA");

    case CL_RGB:
        return ("CL_RGB");

    case CL_RGBA:
        return ("CL_RGBA");

    case CL_BGRA:
        return ("CL_BGRA");

    case CL_ARGB:
        return ("CL_ARGB");

    case CL_INTENSITY:
        return ("CL_INTENSITY");

    case CL_LUMINANCE:
        return ("CL_LUMINANCE");

    case CL_Rx:
        return ("CL_Rx");

    case CL_RGx:
        return ("CL_RGx");

    case CL_RGBx:
        return ("CL_RGBx");

    default:
        return ("Unknown image channel order");
    }
}

std::string imageChannelDataTypeToString(cl_channel_type channelDataType)
{
    switch (channelDataType) {
    case CL_SNORM_INT8:
        return ("CL_SNORM_INT8");

    case CL_SNORM_INT16:
        return ("CL_SNORM_INT16");

    case CL_UNORM_INT8:
        return ("CL_UNORM_INT8");

    case CL_UNORM_INT16:
        return ("CL_UNORM_INT16");

    case CL_UNORM_SHORT_565:
        return ("CL_UNORM_SHORT_565");

    case CL_UNORM_SHORT_555:
        return ("CL_UNORM_SHORT_555");

    case CL_UNORM_INT_101010:
        return ("CL_UNORM_INT_101010");

    case CL_SIGNED_INT8:
        return ("CL_SIGNED_INT8");

    case CL_SIGNED_INT16:
        return ("CL_SIGNED_INT16");

    case CL_SIGNED_INT32:
        return ("CL_SIGNED_INT32");

    case CL_UNSIGNED_INT8:
        return ("CL_UNSIGNED_INT8");

    case CL_UNSIGNED_INT16:
        return ("CL_UNSIGNED_INT16");

    case CL_UNSIGNED_INT32:
        return ("CL_UNSIGNED_INT32");

    case CL_HALF_FLOAT:
        return ("CL_HALF_FLOAT");

    case CL_FLOAT:
        return ("CL_FLOAT");

    default:
        return ("Unknown image channel data type");
    }
}

bool PrintSupportedImageFormats(Context context, cl_mem_flags flag, cl_mem_object_type type)
{
    std::vector<cl::ImageFormat> v;

    context.getSupportedImageFormats(flag, type, // CL_MEM_READ_WRITE,CL_MEM_OBJECT_IMAGE2D,
                                     &v);

    for (int i = 0; i < v.size(); i++) {
        cout << imageChannelOrderToString(v[i].image_channel_order) << ";" <<
            imageChannelDataTypeToString(v[i].image_channel_data_type) << endl;
    }

    return (true);
}
