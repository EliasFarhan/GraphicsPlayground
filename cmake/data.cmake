
function(COPYDATA main_folder exe_name)
file(GLOB_RECURSE DATA_FILES
		"${main_folder}/data/*.json"
		"${main_folder}/data/*.png"
		"${main_folder}/data/*.jpg"
		"${main_folder}/data/*.bmp"
		"${main_folder}/data/*.hdr"
		"${main_folder}/data/*.ktx"
		"${main_folder}/data/*.obj"
		"${main_folder}/data/*.mtl"
		)
		source_group("Data" FILES ${DATA_FILES})
foreach(DATA ${DATA_FILES})
	get_filename_component(FILE_NAME ${DATA} NAME)
	get_filename_component(PATH_NAME ${DATA} DIRECTORY)
	get_filename_component(EXTENSION ${DATA} EXT)
	file(RELATIVE_PATH PATH_NAME "${main_folder}" ${PATH_NAME})
	MESSAGE("Data PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
	set(DATA_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
	MESSAGE("Data OUT PATH: ${DATA_OUTPUT}")
	add_custom_command(
			OUTPUT ${DATA_OUTPUT}
			COMMAND ${CMAKE_COMMAND} -E copy 
			${main_folder}/${PATH_NAME}/${FILE_NAME} 
			${DATA_OUTPUT}
			DEPENDS ${DATA})
	list(APPEND Data_OUTPUT_FILES ${DATA_OUTPUT})
endforeach(DATA)

set(DATA_TARGET "${exe_name}_Data")
MESSAGE(${DATA_TARGET})

add_custom_target(
		"${exe_name}_DATA"
		DEPENDS ${Data_OUTPUT_FILES}
)
endfunction()

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
	set(GLSL_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
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

function(GENERATEGLDATA main_folder exe_name)
COPYDATA(${main_folder} ${exe_name})
CheckGlShader(${main_folder} ${exe_name})
add_dependencies("${exe_name}_DATA" "${exe_name}_ShadersCheck")
endfunction()

function(CheckVKShader main_folder exe_name)

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
	set(GLSL_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}.spv")
	#MESSAGE("GLSL OUT PATH: ${GLSL_OUTPUT}")
	add_custom_command(
			OUTPUT ${GLSL_OUTPUT}
			COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${GLSL_OUTPUT}
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

function(GENERATEVKDATA main_folder exe_name)
COPYDATA(${main_folder} ${exe_name})
CheckVKShader(${main_folder} ${exe_name})
add_dependencies("${exe_name}_DATA" "${exe_name}_ShadersCheck")
endfunction()