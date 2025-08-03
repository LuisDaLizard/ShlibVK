#include "window_internal.h"

shResult_t shWindowCreate(shWindow_t *window, shWindowInfo_t info)
{
    if (!glfwInit())
        return shFail("Failed to initialize GLFW!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, info.resizable);

    GLFWwindow *handle = glfwCreateWindow(info.width, info.height, info.title, NULL, NULL);

    if (!handle)
        return shFail("Failed to create GLFW window!");

    window->handle = handle;

    // TODO: set input callbacks

    return shSuccess();
}

void shWindowDestroy(shWindow_t *window)
{

}

i32_t shWindowGetWidth(shWindow_t *window)
{

}

i32_t shWindowGetHeight(shWindow_t *window)
{

}

b32_t shWindowShouldClose(shWindow_t *window)
{

}