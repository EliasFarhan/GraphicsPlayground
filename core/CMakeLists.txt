
find_package(SDL2 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

file(GLOB_RECURSE CoreFiles src/*cpp include/*.h)
if(TRACY_ENABLE)
list(APPEND CoreFiles "${TRACY_FOLDER}/TracyClient.cpp" "${TRACY_FOLDER}/Tracy.hpp")
endif()
add_library(Core STATIC ${CoreFiles})

target_link_libraries(Core PUBLIC fmt::fmt)
target_include_directories(Core PUBLIC "include/")

target_link_libraries(Core PUBLIC SDL2::SDL2 SDL2::SDL2main spdlog::spdlog spdlog::spdlog_header_only)
set_target_properties(Core PROPERTIES UNITY_BUILD ON)

file(GLOB_RECURSE test_files test/*.cpp)
add_executable(CoreTest ${test_files})
target_link_libraries(CoreTest PRIVATE  GTest::gtest GTest::gtest_main Core)