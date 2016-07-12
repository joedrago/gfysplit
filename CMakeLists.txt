cmake_minimum_required(VERSION 3.6)
project(gfysplit)

# Note: You must download OpenCV 2.4.13 and put it into /opencv
set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/opencv/build")
set(OpenCV_STATIC ON)
find_package(OpenCV REQUIRED)

set(SRCS
    src/main.cpp
)
add_executable(gfysplit
    ${SRCS}
)
target_link_libraries(gfysplit ${OpenCV_LIBS})
set_target_properties(gfysplit PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
set_target_properties(gfysplit PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
set_target_properties(gfysplit PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(gfysplit PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
