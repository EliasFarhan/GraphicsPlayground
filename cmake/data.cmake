
function(CheckGLShader main_folder exe_name)

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
		"${main_folder}/data/*.frag"
		"${main_folder}/data/*.vert"
		"${main_folder}/data/*.tesc"
		"${main_folder}/data/*.tese"
		"${main_folder}/data/*.geom"
		"${main_folder}/data/*.comp"
		)
source_group("Shader Files" FILES ${GLSL_SOURCE_FILES})
foreach(GLSL ${GLSL_SOURCE_FILES})
	get_filename_component(FILE_NAME ${GLSL} NAME)
	get_filename_component(PATH_NAME ${GLSL} DIRECTORY)
	get_filename_component(EXTENSION ${GLSL} EXT)
	file(RELATIVE_PATH PATH_NAME "${main_folder}" ${PATH_NAME})
	#MESSAGE("GLSL PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
	set(GLSL_OUTPUT "${CMAKE_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
	#MESSAGE("GLSL OUT PATH: ${GLSL_OUTPUT}")
	add_custom_command(
			OUTPUT ${GLSL_OUTPUT}
			COMMAND ${GLSL_VALIDATOR} ${GLSL}
			COMMAND ${CMAKE_COMMAND} -E copy 
			${main_folder}/${PATH_NAME}/${FILE_NAME} 
			${GLSL_OUTPUT}
			DEPENDS ${GLSL})
	list(APPEND GLSL_OUTPUT_FILES ${GLSL_OUTPUT})
endforeach(GLSL)

set(SHADER_TARGET "${exe_name}_ShadersCheck")
MESSAGE(${SHADER_TARGET})
add_custom_target(
		"${exe_name}_ShadersCheck"
		DEPENDS ${GLSL_OUTPUT_FILES}
)

endfunction()

function(GENERATEDATA main_folder exe_name)

file(GLOB_RECURSE DATA_FILES
		"${main_folder}/data/*.json"
		"${main_folder}/data/*.png"
		"${main_folder}/data/*.jpg"
		"${main_folder}/data/*.bmp"
		"${main_folder}/data/*.hdr"
		)
		source_group("Shader Files" FILES ${DATA_FILES})
foreach(Data ${DATA_FILES})
	get_filename_component(FILE_NAME ${Data} NAME)
	get_filename_component(PATH_NAME ${Data} DIRECTORY)
	get_filename_component(EXTENSION ${Data} EXT)
	file(RELATIVE_PATH PATH_NAME "${main_folder}" ${PATH_NAME})
	#MESSAGE("Data PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
	set(DATA_OUTPUT "${CMAKE_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
	#MESSAGE("Data OUT PATH: ${Data_OUTPUT}")
	add_custom_command(
			OUTPUT ${Data_OUTPUT}
			COMMAND ${CMAKE_COMMAND} -E copy 
			${main_folder}/${PATH_NAME}/${FILE_NAME} 
			${Data_OUTPUT}
			DEPENDS ${Data})
	list(APPEND Data_OUTPUT_FILES ${Data_OUTPUT})
endforeach(Data)

set(DATA_TARGET "${exe_name}_Data")
MESSAGE(${DATA_TARGET})

add_custom_target(
		"${exe_name}_DATA"
		DEPENDS ${DATA_FILES}
)
CheckGlShader(${main_folder} ${exe_name})
add_dependencies("${exe_name}_DATA" "${exe_name}_ShadersCheck")
endfunction()