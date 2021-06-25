file(GLOB_RECURSE vr_playground_files src/*.cpp include/*.h)

add_executable(vr_playground ${vr_playground_files})
target_include_directories(vr_playground PRIVATE include/)
target_link_libraries(vr_playground PRIVATE CommonGL)
if(WIN32)
target_link_libraries(vr_playground PRIVATE 
	${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/openvr_api.lib)
else()
target_link_libraries(vr_playground PRIVATE 
	${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/openvr_api.so)
endif()
GENERATEGLDATA(${CMAKE_CURRENT_SOURCE_DIR} vr_playground)