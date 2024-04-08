#ifndef SHLIBVK_GRAPHICS_H
#define SHLIBVK_GRAPHICS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Window.h"
#include <stdbool.h>

struct sGraphicsCreateInfo {
    bool debug;

    const char *pAppName;
    const char *pEngineName;

    Window window;
};

struct sGraphics {
    unsigned int windowWidth, windowHeight;
    Window window;

    void *vkInstance;
    void *vkDebugMessenger;
    void *vkPhysicalDevice;
    void *vkDevice;
    void *vkGraphicsQueue, *vkPresentQueue;
    void *vkSurface;
    unsigned int vkGraphicsQueueIndex, vkPresentQueueIndex;

    void *vkSwapChain;
    void **vkSwapChainImages;
    void **vkSwapChainImageViews;
    void **vkSwapChainFramebuffers;
    unsigned int vkSwapChainImageCount;
    unsigned int vkSwapChainImageFormat;
    unsigned int vkSwapChainImageWidth;
    unsigned int vkSwapChainImageHeight;

    void *vkRenderPass;
    void *vkCommandPool;
    void *vkDescriptorPool;
    void *vkCommandBuffer;
    void *vkImageAvailableSemaphore;
    void *vkRenderFinishedSemaphore;
    void *vkInFlightFence;
    unsigned int vkImageIndex;

    void *vkDepthImage;
    void *vkDepthImageMemory;
    void *vkDepthImageView;
};

typedef struct sGraphicsCreateInfo GraphicsCreateInfo;
typedef struct sGraphics *Graphics;

bool GraphicsCreate(GraphicsCreateInfo *pCreateInfo, Graphics *pGraphics);

void GraphicsDestroy(Graphics graphics);

void GraphicsBeginRenderPass(Graphics graphics);

void GraphicsEndRenderPass(Graphics graphics);

void *GraphicsBeginSingleUseCommand(Graphics graphics);

void GraphicsEndSingleUseCommand(Graphics graphics, void *commandBuffer);

#ifdef __cplusplus
}
#endif

#endif //SHLIBVK_GRAPHICS_H
