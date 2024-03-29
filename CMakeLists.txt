cmake_minimum_required (VERSION 3.18)
project(GraphicsPlayground)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /w14640)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor)
endif()

add_compile_definitions(_USE_MATH_DEFINES)

set(TRACY_ENABLE ON CACHE BOOL "")

set_property(GLOBAL PROPERTY USE_FOLDERS On)

if(TRACY_ENABLE)
    add_compile_definitions(TRACY_ENABLE)
    set(TRACY_FOLDER "${CMAKE_SOURCE_DIR}/externals/tracy/" CACHE STRING "")
    include_directories("${TRACY_FOLDER}")
endif()

find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
include(cmake/data.cmake)

add_subdirectory(core)
add_subdirectory(common/gl)
add_subdirectory(common/vulkan)


add_subdirectory(main/gl)
add_subdirectory(main/vk)
add_subdirectory(main/tools)
add_subdirectory(main/vr_playground)
add_subdirectory(main/rt_playground)

