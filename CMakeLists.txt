cmake_minimum_required(VERSION 3.25)
project(ShlibVK C)

option(BUILD_EXAMPLES "Build all example projects" ON)

add_subdirectory(libs/glfw)
find_package(Vulkan COMPONENTS MoltenVK)

set(CMAKE_C_STANDARD 23)

set(SHLIBVK_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
        src/Window.c
        src/Graphics.c
        src/Utils.c
        src/Pipeline.c
        src/Mesh.c
        src/Buffer.c
        src/UniformBuffer.c
        src/Math/Matrix.c
        src/Math/Vec2.c
        src/Math/Vec3.c
        src/Math/Vec4.c
        src/Utilities/List.c
        )

set(INCLUDES
        include/
        ${Vulkan_INCLUDE_DIRS}
        )

set(LIBS
        glfw
        ${Vulkan_LIBRARIES}
        )

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBS})

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()