file(GLOB_RECURSE egj_files src/*.cpp include/*.h)

add_executable(egj ${egj_files})
target_include_directories(egj PRIVATE include/)