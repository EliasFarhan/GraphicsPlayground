file(GLOB_RECURSE source_files include/*.h src/*.cpp)
file(GLOB_RECURSE shaders_files data/*.vert data/*.frag)
add_executable(gl_samples ${source_files} ${shaders_files})
target_include_directories(gl_samples PRIVATE "include/")
target_link_libraries(gl_samples PRIVATE CommonGL)


GENERATEGLDATA(${CMAKE_CURRENT_SOURCE_DIR} gl_samples)

set_target_properties(gl_samples PROPERTIES UNITY_BUILD ON)
set_target_properties (gl_samples PROPERTIES FOLDER GL)
set_target_properties (gl_samples_DATA PROPERTIES FOLDER GL)
set_target_properties (gl_samples_ShadersCheck PROPERTIES FOLDER GL)
if(UNIX)
    IF ( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
    add_custom_command(
            TARGET gl_samples
            POST_BUILD
            COMMAND ${CMAKE_STRIP} gl_samples
    )
    ENDIF()
endif()