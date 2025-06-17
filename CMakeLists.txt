cmake_minimum_required(VERSION 3.28)
project(ModernCPlusPlusDemo VERSION 1.0.0 LANGUAGES CXX)

# 使用 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# Boost 库设置
set(Boost_DEBUG ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# 打印 Boost 相关变量
message(STATUS "BOOST_ROOT: $ENV{BOOST_ROOT}")
message(STATUS "BOOST_LIBRARYDIR: $ENV{BOOST_LIBRARYDIR}")

if(WIN32)
    # Windows 特定设置
    if(DEFINED ENV{BOOST_ROOT})
        set(BOOST_ROOT $ENV{BOOST_ROOT})
    else()
        set(BOOST_ROOT C:/WorkSpace/boost_1_83_0)
    endif()
    if(DEFINED ENV{BOOST_LIBRARYDIR})
        set(BOOST_LIBRARYDIR $ENV{BOOST_LIBRARYDIR})
    else()
        if(MSVC_VERSION GREATER_EQUAL 1920 AND MSVC_VERSION LESS 1930)
            set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib64-msvc-14.2)
        elseif(MSVC_VERSION GREATER_EQUAL 1930 AND MSVC_VERSION LESS 1940)
            set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib64-msvc-14.3)
        else()
            message(WARNING "Unsupported MSVC version: ${MSVC_VERSION}")
        endif()
    endif()
else()
    # Linux 特定设置
    set(Boost_USE_STATIC_LIBS OFF)
endif()

find_package(Boost 1.83 REQUIRED COMPONENTS system filesystem regex iostreams program_options date_time json)

# 如果找到 Boost, 则打印相关信息
if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION = ${Boost_VERSION}")
    message(STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}")
    message(STATUS "Boost_MAJOR_VERSION = ${Boost_MAJOR_VERSION}")
    message(STATUS "Boost_MINOR_VERSION = ${Boost_MINOR_VERSION}")
    message(STATUS "Boost_SUBMINOR_VERSION = ${Boost_SUBMINOR_VERSION}")
    message(STATUS "Boost_LIB_DIAGNOSTIC_DEFINITIONS = ${Boost_LIB_DIAGNOSTIC_DEFINITIONS}")

    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost not found. Please install Boost or set BOOST_ROOT.")
endif()

# OpenCL 配置
if(WIN32)
    # Windows 平台配置
    if(DEFINED ENV{CUDA_PATH})
        # 如果设置了 CUDA 环境变量
        set(OpenCL_INCLUDE_DIR "$ENV{CUDA_PATH}/include")
        set(OpenCL_LIBRARY "$ENV{CUDA_PATH}/lib/x64/OpenCL.lib")
    else()
        # 默认 NVIDIA CUDA 路径
        set(OpenCL_INCLUDE_DIR "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.6/include")
        set(OpenCL_LIBRARY "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.6/lib/x64/OpenCL.lib")
    endif()
else()
    # Linux 平台配置
    # Linux 通常通过包管理器安装 OpenCL
    # 例如 Ubuntu: sudo apt-get install opencl-headers ocl-icd-opencl-dev
    # CentOS: sudo yum install opencl-headers opencl-utils ocl-icd ocl-icd-devel
    find_path(OpenCL_INCLUDE_DIR CL/cl.h
        PATHS
        "/usr/include"
        "/usr/local/include"
        ENV "CPATH"
    )

    find_library(OpenCL_LIBRARY
        NAMES OpenCL
        PATHS
        "/usr/lib"
        "/usr/lib64"
        "/usr/local/lib"
        "/usr/local/lib64"
        ENV "LIBRARY_PATH"
        ENV "LD_LIBRARY_PATH"
    )
endif()

find_package(OpenCL REQUIRED)

if(OpenCL_FOUND)
    message(STATUS "OpenCL_INCLUDE_DIRS = ${OpenCL_INCLUDE_DIRS}")
    message(STATUS "OpenCL_LIBRARIES = ${OpenCL_LIBRARIES}")
    message(STATUS "OpenCL_VERSION_STRING = ${OpenCL_VERSION_STRING}")

    include_directories(${OpenCL_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "OpenCL not found. Please install OpenCL.")
endif()

# 定义子目录列表
set(SUBDIRECTORIES
    StateCharts
    SPDLogEx
    StateMachine
    DesignModes
    ThreadPool
    Subscriber
    OpenCL
)

# 添加所有子目录
foreach(DIR ${SUBDIRECTORIES})
    add_subdirectory(${DIR})
endforeach()

# 创建主程序的可执行文件
add_executable(${PROJECT_NAME} main.cpp)

# 链接库
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${Boost_LIBRARIES}
    ${SUBDIRECTORIES}
    ${OpenCL_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/TemplateClassDemo
    ${Boost_INCLUDE_DIRS}
    ${OpenCL_INCLUDE_DIRS}
)

# GoogleTest 配置
set(GOOGLETEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Test/googletest)
add_subdirectory(${GOOGLETEST_DIR})

# spdlog 配置
set(SPLOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/splog)
add_subdirectory(${SPLOG_DIR})
target_include_directories(SPDLogEx PRIVATE ${SPLOG_DIR}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)

# 单元测试可执行文件
set(TEST_SOURCE_FILES
    Test/SubscriberTest.cpp
    Test/TestCamera.cpp
)

add_executable(gtest_unitTest ${TEST_SOURCE_FILES})

target_link_libraries(gtest_unitTest PUBLIC
    gtest
    gtest_main
    gmock
    gmock_main
    StateCharts
    Subscriber
)
