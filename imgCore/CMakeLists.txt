
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS src/*.cc include/*.h)
add_library(LuxImageCore SHARED ${PROJECT_SOURCES})
target_include_directories(LuxImageCore PUBLIC include)

set(OpenCV_DIR "/home/lux/Downloads/opencv-4.7.0/build/")
FIND_PACKAGE(OpenCV REQUIRED)
if(OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
    target_link_libraries(LuxImageCore ${OpenCV_LIBS})
else(OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV library not found")
endif(OpenCV_FOUND)

# add_executable(LuxDLLTEST test/LuxDLL_test.cc)
# target_link_libraries(LuxDLLTEST LuxDLL)

