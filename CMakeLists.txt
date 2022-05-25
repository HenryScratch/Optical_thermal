cmake_minimum_required(VERSION 3.16)
project(object_tracker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# for code completion engines
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_compile_definitions(FMT)
add_compile_definitions(DEV)

# Set global compiler warnings
if(MSVC)
    add_compile_options(/W3 /WX)
else()
    add_compile_options(-Wall -pedantic)
endif()


find_package( OpenCV REQUIRED )  # install it manually pls


set( PROJECT_SRCS
        src/tracking.cpp
        src/models.cpp
        src/utils/log.cpp
        src/utils/misc.cpp
        src/config.cpp
        src/tracker/kalman_tracker.cpp
        src/tracker/multi_object_tracker.cpp
        src/lib/hungarian.cpp
        src/tracker/countour_finder.cpp
        src/lib/disjoint_set.cpp
        src/tracker/tracker_log.cpp
        src/utils/draw.cpp
        src/utils/perspective_transformer.cpp
        )

add_library(object_tracker_sdk SHARED ${PROJECT_SRCS})
target_include_directories(object_tracker_sdk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(object_tracker_sdk ${CONAN_LIBS} ${OpenCV_LIBS})

add_executable( main main.cpp)
target_include_directories(main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )
target_link_libraries( main PRIVATE object_tracker_sdk)
