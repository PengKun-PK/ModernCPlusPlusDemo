set(SOURCE_FILES
    OpenCLWrapper.cpp
   )
add_library(OpenCL ${SOURCE_FILES})
target_include_directories(OpenCL PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
