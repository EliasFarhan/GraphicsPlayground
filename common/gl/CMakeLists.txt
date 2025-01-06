

find_package(imgui CONFIG REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(gli CONFIG REQUIRED)


file(GLOB_RECURSE CommonGL_FILES src/* include/*)
add_library(CommonGL STATIC ${CommonGL_FILES})
target_link_libraries(CommonGL PUBLIC Core
        GLEW::GLEW
        gli
        imgui::imgui
        assimp::assimp
        ${OPENGL_LIBRARIES})
target_include_directories(CommonGL PUBLIC "include/")
target_include_directories(CommonGL PUBLIC ${Stb_INCLUDE_DIR})
set_target_properties(CommonGL PROPERTIES UNITY_BUILD ON)
set_target_properties (CommonGL PROPERTIES FOLDER GL)