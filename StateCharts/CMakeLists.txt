set(SOURCE_FILES
    Camera.cpp
    DataSource.cpp
   )
add_library(StateCharts ${SOURCE_FILES})
target_include_directories(StateCharts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
