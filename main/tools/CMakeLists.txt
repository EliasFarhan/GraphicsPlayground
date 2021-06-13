find_package(argh CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_executable(obj_to_gltf src/obj_to_gltf.cpp)
target_link_libraries(obj_to_gltf PRIVATE argh assimp::assimp Core)