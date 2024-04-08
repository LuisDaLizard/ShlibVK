cmake_minimum_required(VERSION 3.25)

add_subdirectory(01-Triangle)
add_subdirectory(02-Triangle3D)
add_subdirectory(03-Texture)

file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)