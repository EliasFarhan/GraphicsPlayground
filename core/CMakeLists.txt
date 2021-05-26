
file(GLOB_RECURSE CoreFiles src/*cpp include/*.h)
add_library(Core STATIC ${CoreFiles})

target_link_libraries(Core PUBLIC fmt::fmt)
target_include_directories(Core PUBLIC "include/")
set_target_properties(Core PROPERTIES UNITY_BUILD ON)