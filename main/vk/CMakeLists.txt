file(GLOB_RECURSE source_files include/*.h src/*.cpp)
add_executable(vk_samples ${source_files})
target_include_directories(vk_samples PRIVATE "include/")
target_link_libraries(vk_samples PRIVATE CommonVk)
GENERATEVKDATA(${CMAKE_CURRENT_SOURCE_DIR} vk_samples)
add_dependencies(vk_samples "vk_samples_DATA")

set_target_properties(vk_samples PROPERTIES UNITY_BUILD ON)

if(UNIX)
    IF ( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
    add_custom_command(
            TARGET vk_samples
            POST_BUILD
            COMMAND ${CMAKE_STRIP} vk_samples
    )
    ENDIF()
endif()