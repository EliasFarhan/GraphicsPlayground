find_package(volk CONFIG REQUIRED)

file(GLOB_RECURSE ENGINE_SRC rt_engine/*.cpp rt_engine/*.h)
add_library(rt_engine STATIC ${ENGINE_SRC})
target_include_directories(rt_engine PUBLIC rt_engine/include)
target_link_libraries(rt_engine PUBLIC CommonVk volk::volk volk::volk_headers)
set_target_properties (rt_engine PROPERTIES FOLDER RT)

file(GLOB_RECURSE HELLO_TRIANGLE_SRC hello_triangle/*.cpp hello_triangle/*.h)
add_executable(rt_triangle ${HELLO_TRIANGLE_SRC})
target_link_libraries(rt_triangle PRIVATE rt_engine)
set_target_properties (rt_triangle PROPERTIES FOLDER RT)