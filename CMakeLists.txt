cmake_minimum_required (VERSION 2.16)
project(GPR5300)


set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)

if(MSVC)
	if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
		set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
	else()
		set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
	endif()
elseif(UNIX)
	set(GLSL_VALIDATOR "glslangValidator")
endif()

file(GLOB_RECURSE GLSL_SOURCE_FILES
		"data/*.frag"
		"data/*.vert"
		"data/*.tesc"
		"data/*.tese"
		"data/*.geom"
		"data/*.comp"
		)
source_group("Shader Files" FILES ${GLSL_SOURCE_FILES})
foreach(GLSL ${GLSL_SOURCE_FILES})
	get_filename_component(FILE_NAME ${GLSL} NAME)
	get_filename_component(PATH_NAME ${GLSL} DIRECTORY)
	get_filename_component(EXTENSION ${GLSL} EXT)
	file(RELATIVE_PATH PATH_NAME "${CMAKE_SOURCE_DIR}" ${PATH_NAME})
	#MESSAGE("GLSL PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
	set(GLSL_OUTPUT "${CMAKE_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
	add_custom_command(
			OUTPUT ${GLSL_OUTPUT}
			COMMAND ${GLSL_VALIDATOR} ${GLSL}
			COMMAND ${CMAKE_COMMAND} -E copy 
			${CMAKE_SOURCE_DIR}/${PATH_NAME}/${FILE_NAME} 
			${GLSL_OUTPUT}

			DEPENDS ${GLSL})
	list(APPEND GLSL_OUTPUT_FILES ${GLSL_OUTPUT})
endforeach(GLSL)

add_custom_target(
		ShadersCheck
		DEPENDS ${GLSL_OUTPUT_FILES}
)

file(GLOB_RECURSE SOURCES src/*.cpp include/*.h)
add_library(CommonLib STATIC ${SOURCES} ${GLSL_SOURCE_FILES})
add_dependencies(CommonLib ShadersCheck)
target_include_directories(CommonLib PUBLIC "include/")
target_link_libraries(CommonLib PUBLIC SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(CommonLib PUBLIC imgui::imgui)
target_link_libraries(CommonLib PUBLIC glad::glad)
target_link_libraries(CommonLib PUBLIC ${OPENGL_LIBRARIES})

file(GLOB_RECURSE main_files main/*.cpp)
foreach(test_file ${main_files})
	get_filename_component(test_name ${test_file} NAME_WE)
	add_executable(${test_name} ${test_file})
    target_link_libraries(${test_name} PRIVATE CommonLib)
endforeach()