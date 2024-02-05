cmake_minimum_required(VERSION 3.28)
project(TestCmakeList VERSION 1.0.0)

# C++20 support
SET(SRC_LIST main.cpp)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Boost setup
set(Boost_DEBUG ON)
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT C:/WorkSpace/boost_1_83_0)
set(Boost_LIBRARY_DIRS C:/WorkSpace/boost_1_83_0/stage/lib)

find_package(Boost 1.83
    COMPONENTS
        system
        filesystem
        regex
        iostreams
        program_options
        date_time
    REQUIRED)

if(Boost_FOUND)
    MESSAGE( STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
    MESSAGE( STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")
    MESSAGE( STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}.")
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# custom library and link
add_subdirectory(MathFunctions)
add_subdirectory(StateCharts)
add_subdirectory(SPDLogEx)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} PUBLIC
                      ${Boost_LIBRARIES}
                      MathFunctions
                      StateCharts
                      SPDLogEx)

target_include_directories(${PROJECT_NAME} PUBLIC
                           ${PROJECT_BINARY_DIR}
                           ${PROJECT_SOURCE_DIR}/MathFunctions
                           ${PROJECT_SOURCE_DIR}/StateCharts
                           ${PROJECT_SOURCE_DIR}/SPDLogEx)

# google test
set(GOOGLETEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Test/googletest)
set(gtest_disable_pthreads on)

add_subdirectory(${GOOGLETEST_DIR})

# spLog
set(SPLOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/splog)
add_subdirectory(${SPLOG_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC
                      spdlog::spdlog)

# test exe
set(TEST_SOURCE_FILES
    Test/TestCamera.cpp
   )

add_executable(gtest_unitTest ${TEST_SOURCE_FILES})

target_link_libraries(gtest_unitTest
   gtest
   gtest_main
   gmock
   gmock_main
   StateCharts
   MathFunctions)