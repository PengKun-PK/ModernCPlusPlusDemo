#pragma once

#include <CL/cl.h>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class OpenCLWrapper
{
public:
    // 内核配置结构体
    struct KernelConfig
    {
        std::vector<size_t> globalWorkSize;
        std::vector<size_t> localWorkSize;
        bool useLocalSize = false;
    };

    // 性能统计结构体
    struct PerformanceStats
    {
        double totalTime = 0.0;
        int executionCount = 0;
        double averageTime = 0.0;
        double minTime = std::numeric_limits<double>::max();
        double maxTime = 0.0;
    };

    // 单例模式
    static OpenCLWrapper& getInstance();

    // 禁止拷贝
    OpenCLWrapper(const OpenCLWrapper&) = delete;
    OpenCLWrapper& operator=(const OpenCLWrapper&) = delete;

    // 禁止移动
    OpenCLWrapper(OpenCLWrapper&&) = delete;
    OpenCLWrapper& operator=(OpenCLWrapper&&) = delete;

    // 公共接口
    void init(cl_device_type deviceType = CL_DEVICE_TYPE_GPU);
    void loadKernelsFromDirectory(const std::string& directory);
    void loadKernelFile(const std::string& filename);
    cl_kernel getKernel(const std::string& kernelName);

    void executeKernel(const std::string& kernelName, const KernelConfig& config, std::function<void()> setArgsFunc);

    template<typename T>
    cl_mem createBuffer(size_t size, cl_mem_flags flags);

    template<typename T>
    void writeBuffer(cl_mem buffer, const std::vector<T>& data, bool blocking = true);

    template<typename T>
    void readBuffer(cl_mem buffer, std::vector<T>& data, bool blocking = true);

    void finish();
    const std::unordered_map<std::string, PerformanceStats>& getPerformanceStats() const;

private:
    OpenCLWrapper() = default;
    ~OpenCLWrapper();

    bool m_initialized = false;
    cl_platform_id m_platform = nullptr;
    cl_device_id m_device = nullptr;
    cl_context m_context = nullptr;
    cl_command_queue m_queue = nullptr;
    std::unordered_map<std::string, cl_program> m_programs;
    std::unordered_map<std::string, cl_kernel> m_kernels;
    std::unordered_map<std::string, PerformanceStats> m_performanceStats;

    void buildProgram(const std::string& source, const std::string& filename);
    void recordExecutionTime(const std::string& kernelName, cl_event event);
    void cleanup();
    void printDeviceInfo();
    void checkError(cl_int error, const std::string& message);
};

// 模板函数实现
template<typename T>
cl_mem OpenCLWrapper::createBuffer(size_t size, cl_mem_flags flags)
{
    cl_int err;
    cl_mem buffer = clCreateBuffer(m_context, flags, size * sizeof(T), nullptr, &err);
    checkError(err, "Failed to create buffer");
    return buffer;
}

template<typename T>
void OpenCLWrapper::writeBuffer(cl_mem buffer, const std::vector<T>& data, bool blocking)
{
    cl_int err = clEnqueueWriteBuffer(m_queue, buffer, blocking ? CL_TRUE : CL_FALSE, 0, data.size() * sizeof(T),
                                      data.data(), 0, nullptr, nullptr);
    checkError(err, "Failed to write buffer");
}

template<typename T>
void OpenCLWrapper::readBuffer(cl_mem buffer, std::vector<T>& data, bool blocking)
{
    cl_int err = clEnqueueReadBuffer(m_queue, buffer, blocking ? CL_TRUE : CL_FALSE, 0, data.size() * sizeof(T),
                                     data.data(), 0, nullptr, nullptr);
    checkError(err, "Failed to read buffer");
}
