#ifndef SHLIBVK_GRAPHICS_H
#define SHLIBVK_GRAPHICS_H

#include "Window.h"
#include <stdbool.h>

struct sGraphicsCreateInfo
{
    bool debug;

    const char *pAppName;
    const char *pEngineName;

    Window window;
};

struct sGraphics
{
    void *vkInstance;
    void *vkDebugMessenger;
    void *vkPhysicalDevice;
    void *vkDevice;
    void *vkGraphicsQueue, *vkPresentQueue;
    void *vkSurface;

    void *vkSwapChain;
    void **vkSwapChainImages;
    void **vkSwapChainImageViews;
    unsigned int vkSwapChainImageCount;
    unsigned int vkSwapChainImageFormat;
    unsigned int vkSwapChainImageWidth;
    unsigned int vkSwapChainImageHeight;

    void *vkRenderPass;

    void *vkShaderCompiler;
};

typedef struct sGraphicsCreateInfo GraphicsCreateInfo;
typedef struct sGraphics *Graphics;

bool GraphicsCreate(GraphicsCreateInfo *pCreateInfo, Graphics *pGraphics);
void GraphicsDestroy(Graphics graphics);

#endif //SHLIBVK_GRAPHICS_H
