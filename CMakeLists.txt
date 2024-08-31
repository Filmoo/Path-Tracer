# CMakeLists.txt

cmake_minimum_required(VERSION 3.12)

project(RayTracing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -g")
endif()

set(SOURCE_DIR src)
set(HEADER_DIR include)

# Add your source files
file(GLOB SOURCE_FILES "${SOURCE_DIR}/*.cc" "${SOURCE_DIR}/*.cpp")

# Add your header files
file(GLOB HEADER_FILES "${HEADER_DIR}/*.hh" "${HEADER_DIR}/*.h")

# Add executable target
add_executable(main ${SOURCE_FILES} ${HEADER_FILES})

include_directories(${HEADER_DIR})

target_link_libraries(main d3d11.lib)
