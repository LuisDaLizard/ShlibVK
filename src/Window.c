#include "ShlibVK/Window.h"

#include <stdlib.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

bool WindowCreate(WindowCreateInfo *pCreateInfo, Window *pWindow)
{
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *handle = glfwCreateWindow(pCreateInfo->width, pCreateInfo->height, pCreateInfo->pTitle, NULL, NULL);
    if (!handle)
        return false;

    Window window = malloc(sizeof(struct sWindow));
    window->width = pCreateInfo->width;
    window->height = pCreateInfo->height;
    window->pHandle = handle;

    *pWindow = window;

    return true;
}

void WindowDestroy(Window window)
{
    if (!window)
        return;

    glfwDestroyWindow(window->pHandle);
    free(window);
}

bool WindowShouldClose(Window window)
{
    return glfwWindowShouldClose(window->pHandle);
}

void WindowPollEvents(Window window)
{
    glfwPollEvents();
}