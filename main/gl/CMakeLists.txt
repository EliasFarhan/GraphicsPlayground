file(GLOB_RECURSE source_files include/*.h src/*.cpp)
add_executable(gl_samples ${source_files})
target_include_directories(gl_samples PRIVATE "include/")
target_link_libraries(gl_samples PRIVATE CommonGL)
GENERATEGLDATA(${CMAKE_CURRENT_SOURCE_DIR} gl_samples)
add_dependencies(gl_samples "gl_samples_DATA")

set_target_properties(gl_samples PROPERTIES UNITY_BUILD ON)