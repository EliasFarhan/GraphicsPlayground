cmake_minimum_required (VERSION 2.16)
project(GPR5300)


set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(_USE_MATH_DEFINES)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
include(cmake/data.cmake)

add_subdirectory(core)
add_subdirectory(common/gl)
add_subdirectory(common/vulkan)


add_subdirectory(main/gl)
