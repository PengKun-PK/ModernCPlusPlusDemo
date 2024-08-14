set(SOURCE_FILES
    SPDLogEx.cpp
    ILogger.cpp
   )
add_library(SPDLogEx ${SOURCE_FILES})
target_include_directories(SPDLogEx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
