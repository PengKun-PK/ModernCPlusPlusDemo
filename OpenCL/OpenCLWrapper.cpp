#include "OpenCLWrapper.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

OpenCLWrapper& OpenCLWrapper::getInstance()
{
    static OpenCLWrapper instance;
    return instance;
}

void OpenCLWrapper::init(cl_device_type deviceType)
{
    if (m_initialized)
        return;

    cl_int err;

    // 获取所有平台
    cl_uint platformCount;
    err = clGetPlatformIDs(0, nullptr, &platformCount);
    checkError(err, "Failed to get platform count");

    std::vector<cl_platform_id> platforms(platformCount);
    err = clGetPlatformIDs(platformCount, platforms.data(), nullptr);
    checkError(err, "Failed to get platforms");

    // 选择最适合的平台和设备
    for (const auto& platform : platforms)
    {
        cl_uint deviceCount;
        err = clGetDeviceIDs(platform, deviceType, 0, nullptr, &deviceCount);
        if (err == CL_SUCCESS && deviceCount > 0)
        {
            m_platform = platform;
            err = clGetDeviceIDs(platform, deviceType, 1, &m_device, nullptr);
            if (err == CL_SUCCESS)
                break;
        }
    }

    if (!m_device && deviceType == CL_DEVICE_TYPE_GPU)
    {
        // GPU不可用时尝试CPU
        init(CL_DEVICE_TYPE_CPU);
        return;
    }

    checkError(m_device ? CL_SUCCESS : CL_DEVICE_NOT_FOUND, "No suitable device found");

    // 创建上下文和命令队列
    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    checkError(err, "Failed to create context");

    cl_queue_properties properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
    m_queue = clCreateCommandQueueWithProperties(m_context, m_device, properties, &err);
    checkError(err, "Failed to create command queue");

    m_initialized = true;
    printDeviceInfo();
}

void OpenCLWrapper::loadKernelsFromDirectory(const std::string& directory)
{
    std::filesystem::path sourceDir = std::filesystem::path(__FILE__).parent_path().parent_path();
    std::filesystem::path kernelsDir = sourceDir / directory;

    if (!std::filesystem::exists(kernelsDir))
    {
        throw std::runtime_error("Kernels directory not found: " + kernelsDir.string());
    }

    // 遍历加载内核文件
    for (const auto& entry : std::filesystem::directory_iterator(kernelsDir))
    {
        if (entry.path().extension() == ".cl")
        {
            std::cout << "Loading kernel: " << entry.path() << std::endl;
            loadKernelFile(entry.path().string());
        }
    }
}

void OpenCLWrapper::loadKernelFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open kernel file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    buildProgram(source, filename);
}

cl_kernel OpenCLWrapper::getKernel(const std::string& kernelName)
{
    auto it = m_kernels.find(kernelName);
    if (it == m_kernels.end())
    {
        throw std::runtime_error("Kernel not found: " + kernelName);
    }
    return it->second;
}

void OpenCLWrapper::executeKernel(const std::string& kernelName, const KernelConfig& config,
                                  std::function<void()> setArgsFunc)
{
    auto kernel = getKernel(kernelName);

    // 设置参数
    setArgsFunc();

    // 获取设备的最大工作组大小
    size_t maxWorkGroupSize;
    cl_int err =
        clGetDeviceInfo(m_device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkGroupSize), &maxWorkGroupSize, nullptr);
    checkError(err, "Failed to get max work group size");

    // 调整工作组大小
    std::vector<size_t> adjustedGlobalSize = config.globalWorkSize;
    std::vector<size_t> adjustedLocalSize = config.localWorkSize;

    if (config.useLocalSize)
    {
        // 确保局部大小不超过设备限制
        if (adjustedLocalSize[0] > maxWorkGroupSize)
        {
            adjustedLocalSize[0] = maxWorkGroupSize;
        }

        // 确保全局大小是局部大小的倍数
        adjustedGlobalSize[0] =
            ((adjustedGlobalSize[0] + adjustedLocalSize[0] - 1) / adjustedLocalSize[0]) * adjustedLocalSize[0];
    }

    // 创建事件
    cl_event event;

    // 正确调用 clEnqueueNDRangeKernel
    err = clEnqueueNDRangeKernel(m_queue,                    // 命令队列
                                 kernel,                     // 内核
                                 1,                          // 工作维度（1D数组）
                                 nullptr,                    // 全局偏移量
                                 adjustedGlobalSize.data(),  // 全局工作项数量
                                 config.useLocalSize ? adjustedLocalSize.data() : nullptr,  // 局部工作组大小
                                 0,                                                         // 等待事件数量
                                 nullptr,                                                   // 等待事件列表
                                 &event                                                     // 输出事件
    );

    checkError(err, "Failed to execute kernel: " + kernelName);

    // 等待执行完成
    err = clWaitForEvents(1, &event);
    checkError(err, "Failed to wait for kernel execution");

    // 记录执行时间
    recordExecutionTime(kernelName, event);
    clReleaseEvent(event);
}

void OpenCLWrapper::finish()
{
    clFinish(m_queue);
}

const std::unordered_map<std::string, OpenCLWrapper::PerformanceStats>& OpenCLWrapper::getPerformanceStats() const
{
    return m_performanceStats;
}

OpenCLWrapper::~OpenCLWrapper()
{
    cleanup();
}

void OpenCLWrapper::buildProgram(const std::string& source, const std::string& filename)
{
    cl_int err;

    // 创建程序
    const char* sourceCStr = source.c_str();
    m_programs[filename] = clCreateProgramWithSource(m_context, 1, &sourceCStr, nullptr, &err);
    checkError(err, "Failed to create program from " + filename);

    // 编译程序
    err = clBuildProgram(m_programs[filename], 1, &m_device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        // 获取编译错误信息
        size_t logSize;
        clGetProgramBuildInfo(m_programs[filename], m_device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::vector<char> log(logSize);
        clGetProgramBuildInfo(m_programs[filename], m_device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
        throw std::runtime_error("Failed to build program " + filename + ": " + std::string(log.data()));
    }

    // 创建所有内核
    cl_uint numKernels;
    err = clCreateKernelsInProgram(m_programs[filename], 0, nullptr, &numKernels);
    checkError(err, "Failed to get kernel count from " + filename);

    std::vector<cl_kernel> kernels(numKernels);
    err = clCreateKernelsInProgram(m_programs[filename], numKernels, kernels.data(), nullptr);
    checkError(err, "Failed to create kernels from " + filename);

    // 获取内核名称并存储
    for (const auto& kernel : kernels)
    {
        size_t nameSize;
        err = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, 0, nullptr, &nameSize);
        checkError(err, "Failed to get kernel name size");

        std::vector<char> nameBuffer(nameSize);
        err = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, nameSize, nameBuffer.data(), nullptr);
        checkError(err, "Failed to get kernel name");

        std::string kernelName(nameBuffer.data());
        m_kernels[kernelName] = kernel;
    }
}

void OpenCLWrapper::recordExecutionTime(const std::string& kernelName, cl_event event)
{
    cl_ulong start, end;
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(start), &start, nullptr);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(end), &end, nullptr);

    double timeInSeconds = (end - start) * 1.0e-9;
    auto& stats = m_performanceStats[kernelName];
    stats.totalTime += timeInSeconds;
    stats.executionCount++;
    stats.averageTime = stats.totalTime / stats.executionCount;
    stats.minTime = std::min(stats.minTime, timeInSeconds);
    stats.maxTime = std::max(stats.maxTime, timeInSeconds);
}

void OpenCLWrapper::cleanup()
{
    for (auto& kernel : m_kernels)
    {
        clReleaseKernel(kernel.second);
    }
    for (auto& program : m_programs)
    {
        clReleaseProgram(program.second);
    }
    if (m_queue)
        clReleaseCommandQueue(m_queue);
    if (m_context)
        clReleaseContext(m_context);
}

void OpenCLWrapper::printDeviceInfo()
{
    char buffer[1024];
    clGetDeviceInfo(m_device, CL_DEVICE_NAME, sizeof(buffer), buffer, nullptr);
    std::cout << "Device: " << buffer << std::endl;

    clGetDeviceInfo(m_device, CL_DEVICE_VENDOR, sizeof(buffer), buffer, nullptr);
    std::cout << "Vendor: " << buffer << std::endl;

    clGetDeviceInfo(m_device, CL_DEVICE_VERSION, sizeof(buffer), buffer, nullptr);
    std::cout << "Version: " << buffer << std::endl;

    cl_uint compute_units;
    clGetDeviceInfo(m_device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, nullptr);
    std::cout << "Compute units: " << compute_units << std::endl;
}

void OpenCLWrapper::checkError(cl_int error, const std::string& message)
{
    if (error != CL_SUCCESS)
    {
        throw std::runtime_error(message + " (Error code: " + std::to_string(error) + ")");
    }
}
