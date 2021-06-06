

find_package(imgui CONFIG REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(assimp CONFIG REQUIRED)


file(GLOB_RECURSE CommonGL_FILES src/* include/*)

list(APPEND CommonGL_FILES "${TRACY_FOLDER}/TracyOpenGL.hpp")
add_library(CommonGL STATIC ${CommonGL_FILES})
target_link_libraries(CommonGL PUBLIC imgui::imgui)
target_link_libraries(CommonGL PUBLIC GLEW::GLEW)
target_link_libraries(CommonGL PUBLIC assimp::assimp)
target_link_libraries(CommonGL PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(CommonGL PUBLIC Core)
target_include_directories(CommonGL PUBLIC "include/")
target_include_directories(CommonGL PUBLIC ${STB_INCLUDE_DIRS})
set_target_properties(CommonGL PROPERTIES UNITY_BUILD ON)