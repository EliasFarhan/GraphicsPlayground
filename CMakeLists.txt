cmake_minimum_required (VERSION 2.16)
project(GPR5300)


set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
include(cmake/data.cmake)

add_subdirectory(core)
add_subdirectory(common/gl)
add_subdirectory(common/vulkan)

file(GLOB main_folder "main/gl/*")
foreach(application_folder ${main_folder})
	if(IS_DIRECTORY ${application_folder})
		get_filename_component(exe_name ${application_folder} NAME_WE)
		file(GLOB exe_file ${application_folder}/*.cpp)
		GENERATEDATA(${application_folder} ${exe_name})
		
		add_executable(${exe_name} ${exe_file})
		add_dependencies(${exe_name} "${exe_name}_DATA")
		target_link_libraries(${exe_name} PRIVATE CommonGL)
	endif()
endforeach()