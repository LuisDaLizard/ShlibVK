cmake_minimum_required(VERSION 3.25)
project(02-Quad3D C)

set(CMAKE_C_STANDARD 90)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} ShlibVK)
target_include_directories(${PROJECT_NAME} PRIVATE ${SHLIBVK_INCLUDE})