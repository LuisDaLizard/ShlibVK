#include "ShlibVK/Window.h"

#include <stdlib.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void WindowResizeCallback(GLFWwindow *handle, int width, int height)
{
    Window window = (Window) glfwGetWindowUserPointer(handle);
    window->width = width;
    window->height = height;

    if (window->resizeCallback)
        window->resizeCallback(window->pUserData, width, height);
}

bool WindowCreate(WindowCreateInfo *pCreateInfo, Window *pWindow)
{
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *handle = glfwCreateWindow(pCreateInfo->width, pCreateInfo->height, pCreateInfo->pTitle, NULL, NULL);
    if (!handle)
        return false;

    Window window = malloc(sizeof(struct sWindow));
    window->width = pCreateInfo->width;
    window->height = pCreateInfo->height;
    window->pHandle = handle;
    window->pUserData = pCreateInfo->pUserData;
    window->resizeCallback = pCreateInfo->resizeCallback;

    glfwSetWindowUserPointer(handle, window);
    glfwSetFramebufferSizeCallback(handle, WindowResizeCallback);

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