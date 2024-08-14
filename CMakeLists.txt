cmake_minimum_required(VERSION 3.28)
project(ModernCPlusPlusDemo VERSION 1.0.0 LANGUAGES CXX)

# 使用 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Boost 库设置
set(Boost_DEBUG ON)
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT C:/WorkSpace/boost_1_83_0)
find_package(Boost 1.83 REQUIRED COMPONENTS system filesystem regex iostreams program_options date_time json)

# 如果找到 Boost, 则打印相关信息
if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
    message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")
    message(STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}.")

    include_directories(${Boost_INCLUDE_DIRS})
endif()

# 定义子目录列表
set(SUBDIRECTORIES
    MathFunctions
    StateCharts
    SPDLogEx
    StateMachine
    DesignModes
    ThreadPool
    Subscriber
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
)

target_include_directories(${PROJECT_NAME} PUBLIC
                           ${PROJECT_BINARY_DIR}
                           ${PROJECT_SOURCE_DIR}/TemplateClassDemo)

# GoogleTest 配置
set(GOOGLETEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Test/googletest)
add_subdirectory(${GOOGLETEST_DIR})

# spdlog 配置
set(SPLOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/splog)
add_subdirectory(${SPLOG_DIR})
target_include_directories(SPDLogEx PRIVATE ${SPLOG_DIR}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)

# 单元测试可执行文件
set(TEST_SOURCE_FILES Test/TestCamera.cpp)
add_executable(gtest_unitTest ${TEST_SOURCE_FILES})
target_link_libraries(gtest_unitTest PUBLIC
                      gtest
                      gtest_main
                      gmock
                      gmock_main
                      StateCharts
                      MathFunctions)
