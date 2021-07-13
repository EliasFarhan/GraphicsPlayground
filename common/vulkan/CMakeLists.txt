
find_package(imgui CONFIG REQUIRED)


file(GLOB_RECURSE CommonVk_FILES src/* include/*)
add_library(CommonVk STATIC ${CommonVk_FILES})
target_link_libraries(CommonVk PUBLIC imgui::imgui)
target_link_libraries(CommonVk PUBLIC Core)
target_include_directories(CommonVk PUBLIC "include/")
target_include_directories(CommonVk PUBLIC ${STB_INCLUDE_DIRS})
set_target_properties(CommonVk PROPERTIES UNITY_BUILD ON)

set_target_properties (CommonVk PROPERTIES FOLDER VK)