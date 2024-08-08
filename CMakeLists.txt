# CMakeLists.txt

cmake_minimum_required(VERSION 3.12)

project(RayTracing)

set(CMAKE_CXX_COMPILER g++)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "-Wall -g ")

set(SOURCE_DIR src)

# Add your source files
file(GLOB SOURCE_FILES "${SOURCE_DIR}/*.cc")

# Add your header files
file(GLOB HEADER_FILES "${SOURCE_DIR}/*.hh")

# Add executable target
add_executable(main src/scene.cc ${SOURCE_FILES} ${HEADER_FILES} src/plane.cc src/box.cc src/triangle.cc src/utils.cc src/tiny_obj_loader.cc)