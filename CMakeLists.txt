cmake_minimum_required(VERSION 3.25)
project(ShlibVK C)

option(BUILD_EXAMPLES "Build all example projects" ON)

add_subdirectory(libs/glfw)
find_package(Vulkan COMPONENTS shaderc_combined)

set(CMAKE_C_STANDARD 90)

set(SHLIBVK_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
        src/Window.c
        src/Graphics.c
        src/Utils.c
        src/Pipeline.c
        src/Mesh.c
        )

set(INCLUDES
        include/
        ${Vulkan_INCLUDE_DIRS}
        )

set(LIBS
        glfw
        ${Vulkan_LIBRARIES}
        ${Vulkan_shaderc_combined_LIBRARY}
        )

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBS})

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()