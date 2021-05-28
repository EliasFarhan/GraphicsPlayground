
find_package(SDL2 CONFIG REQUIRED)
file(GLOB_RECURSE CoreFiles src/*cpp include/*.h)
add_library(Core STATIC ${CoreFiles})

target_link_libraries(Core PUBLIC fmt::fmt)
target_include_directories(Core PUBLIC "include/")

target_link_libraries(Core PUBLIC SDL2::SDL2 SDL2::SDL2main)
set_target_properties(Core PROPERTIES UNITY_BUILD ON)