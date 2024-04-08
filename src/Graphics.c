#include "ShlibVK/Graphics.h"
#include "ShlibVK/Utils.h"
#include "ShlibVK/Window.h"
#include "ShlibVK/Texture.h"


#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <shaderc/shaderc.h>

#include <stdlib.h>
#include <string.h>

const char *ValidationLayers[] = { "VK_LAYER_KHRONOS_validation" };
#ifdef WIN32
const unsigned int RequiredDeviceExtensionCount = 1;
const char *RequiredDeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#endif
#ifdef __APPLE__
const unsigned int RequiredDeviceExtensionCount = 2;
const char *RequiredDeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset"};
#endif

typedef struct sQueueFamilyIndices
{
    unsigned int graphics;
    unsigned int present;

    bool hasGraphics;
    bool hasPresent;
} QueueFamilyIndices;

typedef struct sSwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;

    unsigned int formatCount;
    VkSurfaceFormatKHR *pFormats;

    unsigned int presentModeCount;
    VkPresentModeKHR *pPresentModes;
} SwapChainSupportDetails;

const char **GetRequiredExtensions(GraphicsCreateInfo *pCreateInfo, unsigned int *pExtensionCount);
int RateDeviceSuitability(Graphics graphics, VkPhysicalDevice device);
bool CheckDeviceExtensionSuitability(VkPhysicalDevice device);
SwapChainSupportDetails QuerySwapChainSupport(Graphics graphics, VkPhysicalDevice device);
QueueFamilyIndices FindQueueFamilies(Graphics graphics, VkPhysicalDevice device);
VkSurfaceFormatKHR ChooseSwapSurfaceFormat(VkSurfaceFormatKHR *pAvailableFormats, unsigned int availableFormatCount);
VkPresentModeKHR ChooseSwapPresentMode(VkPresentModeKHR *pAvailableModes, unsigned int availableModeCount);
VkExtent2D ChooseSwapExtent(Window window, VkSurfaceCapabilitiesKHR capabilities);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
VkFormat FindSupportedFormat(Graphics graphics, unsigned int formatCount, VkFormat *pFormats, VkImageTiling tiling, VkFormatFeatureFlags features);
unsigned int FindMemoryType(Graphics graphics, unsigned int typeFilter, VkMemoryPropertyFlags properties);

void CreateInstance(GraphicsCreateInfo *pCreateInfo, Graphics graphics);
void CreateDebugMessenger(Graphics graphics);
void PickPhysicalDevice(GraphicsCreateInfo *pCreateInfo, Graphics graphics);
void CreateLogicalDevice(GraphicsCreateInfo *pCreateInfo, Graphics graphics);
void CreateSurface(Graphics graphics);
void CreateSwapChain(Graphics graphics);
void CreateImageViews(Graphics graphics);
void CreateRenderPass(Graphics graphics);
void CreateFramebuffers(Graphics graphics);
void CreateCommandPool(Graphics graphics);
void CreateCommandBuffer(Graphics graphics);
void CreateSyncObjects(Graphics graphics);
void RecreateSwapChain(Graphics graphics);
void CleanupSwapChain(Graphics graphics);
void CreateDepthResources(Graphics graphics);
void CreateDescriptorPool(Graphics graphics);

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

bool GraphicsCreate(GraphicsCreateInfo *pCreateInfo, Graphics *pGraphics)
{
    Graphics graphics = malloc(sizeof(struct sGraphics));
    graphics->window = pCreateInfo->window;
    graphics->windowWidth = graphics->window->width;
    graphics->windowHeight = graphics->window->height;

    CreateInstance(pCreateInfo, graphics);
    if (pCreateInfo->debug)
        CreateDebugMessenger(graphics);
    CreateSurface(graphics);
    PickPhysicalDevice(pCreateInfo, graphics);
    CreateLogicalDevice(pCreateInfo, graphics);
    CreateSwapChain(graphics);
    CreateImageViews(graphics);
    CreateRenderPass(graphics);
    CreateCommandPool(graphics);
    CreateDepthResources(graphics);
    CreateFramebuffers(graphics);
    CreateCommandBuffer(graphics);
    CreateDescriptorPool(graphics);
    CreateSyncObjects(graphics);

    *pGraphics = graphics;
    return true;
}

void GraphicsDestroy(Graphics graphics)
{
    if (!graphics)
        return;

    vkDeviceWaitIdle(graphics->vkDevice);

    CleanupSwapChain(graphics);

    vkDestroyDescriptorPool(graphics->vkDevice, graphics->vkDescriptorPool, NULL);

    vkDestroySemaphore(graphics->vkDevice, graphics->vkImageAvailableSemaphore, NULL);
    vkDestroySemaphore(graphics->vkDevice, graphics->vkRenderFinishedSemaphore, NULL);
    vkDestroyFence(graphics->vkDevice, graphics->vkInFlightFence, NULL);

    vkDestroyCommandPool(graphics->vkDevice, graphics->vkCommandPool, NULL);
    vkDestroyRenderPass(graphics->vkDevice, graphics->vkRenderPass, NULL);
    vkDestroyDevice(graphics->vkDevice, NULL);
    vkDestroySurfaceKHR(graphics->vkInstance, graphics->vkSurface, NULL);
    if (graphics->vkDebugMessenger)
        DestroyDebugUtilsMessengerEXT(graphics->vkInstance, graphics->vkDebugMessenger, NULL);
    vkDestroyInstance(graphics->vkInstance, NULL);
    free(graphics);
}

void GraphicsBeginRenderPass(Graphics graphics)
{
    vkWaitForFences(graphics->vkDevice, 1, (VkFence *)&graphics->vkInFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(graphics->vkDevice, 1, (VkFence *)&graphics->vkInFlightFence);

    VkResult result = vkAcquireNextImageKHR(graphics->vkDevice, graphics->vkSwapChain, UINT64_MAX, graphics->vkImageAvailableSemaphore, VK_NULL_HANDLE, &graphics->vkImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain(graphics);
        result = vkAcquireNextImageKHR(graphics->vkDevice, graphics->vkSwapChain, UINT64_MAX, graphics->vkImageAvailableSemaphore, VK_NULL_HANDLE, &graphics->vkImageIndex);
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        WriteError(1, "Failed to acquire swap chain image");

    vkResetCommandBuffer(graphics->vkCommandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo = { 0 };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    result = vkBeginCommandBuffer(graphics->vkCommandBuffer, &beginInfo);
    if (result != VK_SUCCESS)
        WriteError(1, "Failed to begin recording command buffer");

    VkClearValue clearValues[2];
    clearValues[0].color = (VkClearColorValue){{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = (VkClearDepthStencilValue){1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo = { 0 };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = graphics->vkRenderPass;
    renderPassInfo.framebuffer = graphics->vkSwapChainFramebuffers[graphics->vkImageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent.width = graphics->vkSwapChainImageWidth;
    renderPassInfo.renderArea.extent.height = graphics->vkSwapChainImageHeight;
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(graphics->vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)graphics->vkSwapChainImageWidth;
    viewport.height = (float)graphics->vkSwapChainImageHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(graphics->vkCommandBuffer, 0, 1, &viewport);

    VkRect2D scissor = { 0 };
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent.width = graphics->vkSwapChainImageWidth;
    scissor.extent.height = graphics->vkSwapChainImageHeight;
    vkCmdSetScissor(graphics->vkCommandBuffer, 0, 1, &scissor);
}

void GraphicsEndRenderPass(Graphics graphics)
{
    vkCmdEndRenderPass(graphics->vkCommandBuffer);

    if (vkEndCommandBuffer(graphics->vkCommandBuffer) != VK_SUCCESS)
        WriteError(1, "Failed to record command buffer");

    //TODO: Consider moving submit process
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = (VkSemaphore *)&graphics->vkImageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = (VkCommandBuffer *)&graphics->vkCommandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = (VkSemaphore *)&graphics->vkRenderFinishedSemaphore;

    if (vkQueueSubmit(graphics->vkGraphicsQueue, 1, &submitInfo, graphics->vkInFlightFence) != VK_SUCCESS)
        WriteError(1, "Failed to submit draw command buffer");

    VkPresentInfoKHR presentInfo = { 0 };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = (VkSemaphore *)&graphics->vkRenderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = (VkSwapchainKHR *)&graphics->vkSwapChain;
    presentInfo.pImageIndices = &graphics->vkImageIndex;
    presentInfo.pResults = NULL; // Optional

    VkResult result = vkQueuePresentKHR(graphics->vkPresentQueue, &presentInfo);

    bool resize = false;

    resize = graphics->window->width != graphics->windowWidth || graphics->window->height != graphics->windowHeight;

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resize)
        RecreateSwapChain(graphics);
    else if (result != VK_SUCCESS)
        WriteError(1, "Failed to present swap chain image");
}

void *GraphicsBeginSingleUseCommand(Graphics graphics)
{
    VkCommandBufferAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = graphics->vkCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(graphics->vkDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = { 0 };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void GraphicsEndSingleUseCommand(Graphics graphics, void *commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = (VkCommandBuffer *)&commandBuffer;

    vkQueueSubmit(graphics->vkGraphicsQueue, 1, &submitInfo, NULL);
    vkQueueWaitIdle(graphics->vkGraphicsQueue);

    vkFreeCommandBuffers(graphics->vkDevice, graphics->vkCommandPool, 1, (VkCommandBuffer *)&commandBuffer);
}

void CreateInstance(GraphicsCreateInfo *pCreateInfo, Graphics graphics)
{
    VkApplicationInfo appInfo = { 0 };
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = pCreateInfo->pAppName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = pCreateInfo->pEngineName;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.ppEnabledExtensionNames = GetRequiredExtensions(pCreateInfo, &createInfo.enabledExtensionCount);
    createInfo.enabledLayerCount = 0;
    if (pCreateInfo->debug)
    {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = ValidationLayers;
    }
#ifdef __APPLE__
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VkResult result = vkCreateInstance(&createInfo, NULL, (VkInstance*)&graphics->vkInstance);
    if (result != VK_SUCCESS)
        WriteError(1, "Unable to create Vulkan instance!");
}

void CreateDebugMessenger(Graphics graphics)
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = graphics;

    VkResult result = CreateDebugUtilsMessengerEXT(graphics->vkInstance, &createInfo, NULL, (VkDebugUtilsMessengerEXT *)&graphics->vkDebugMessenger);
    if (result != VK_SUCCESS)
        WriteError(1, "Unable to set up debug messenger");
}

void PickPhysicalDevice(GraphicsCreateInfo *pCreateInfo, Graphics graphics)
{
    unsigned int deviceCount = 0;
    vkEnumeratePhysicalDevices(graphics->vkInstance, &deviceCount, NULL);

    if (!deviceCount)
        WriteError(1, "Failed to find supported GPU");

    VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(graphics->vkInstance, &deviceCount, devices);

    VkPhysicalDevice bestDevice = devices[0];
    int bestScore = RateDeviceSuitability(graphics, devices[0]);
    int i;
    for (i = 1; i < deviceCount; i++)
    {
        int score = RateDeviceSuitability(graphics, devices[i]);

        if (score > bestScore)
        {
            bestDevice = devices[i];
            bestScore = score;
        }
    }

    if (!bestScore)
        WriteError(1, "Unable to find suitable GPU");

    graphics->vkPhysicalDevice = bestDevice;

    free(devices);
}

void CreateLogicalDevice(GraphicsCreateInfo *pCreateInfo, Graphics graphics)
{
    QueueFamilyIndices indices = FindQueueFamilies(graphics, graphics->vkPhysicalDevice);
    float queuePriority = 1.0f;

    unsigned int queueFamilyCount = 2;
    unsigned int queueFamilies[2] = {indices.graphics, indices.present};
    unsigned int uniqueQueueFamilyCount = 0;
    unsigned int *uniqueQueueFamilies = calloc(2, sizeof(unsigned int));

    int i, j;
    for (i = 0; i < queueFamilyCount; i++)
    {
        bool unique = true;

        for (j = 0; j < uniqueQueueFamilyCount; j++)
        {
            if (queueFamilies[i] == uniqueQueueFamilies[j])
            {
                unique = false;
                break;
            }
        }

        if (unique)
        {
            uniqueQueueFamilies[uniqueQueueFamilyCount] = queueFamilies[i];
            uniqueQueueFamilyCount ++;
        }
    }

    VkDeviceQueueCreateInfo *queueCreateInfos = malloc(sizeof(VkDeviceQueueCreateInfo) * uniqueQueueFamilyCount);

    for (i = 0; i < uniqueQueueFamilyCount; i++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = { 0 };
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = { 0 };
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.tessellationShader = VK_TRUE;

    VkDeviceCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = uniqueQueueFamilyCount;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = RequiredDeviceExtensionCount;
    createInfo.ppEnabledExtensionNames = RequiredDeviceExtensions;

    VkResult result = vkCreateDevice(graphics->vkPhysicalDevice, &createInfo, NULL, (VkDevice *)&graphics->vkDevice);
    if (result != VK_SUCCESS)
        WriteError(1, "Unable to create logical device");

    graphics->vkGraphicsQueueIndex = indices.graphics;
    graphics->vkPresentQueueIndex = indices.present;

    vkGetDeviceQueue(graphics->vkDevice, indices.graphics, 0, (VkQueue *)&graphics->vkGraphicsQueue);
    vkGetDeviceQueue(graphics->vkDevice, indices.present, 0, (VkQueue *)&graphics->vkPresentQueue);
}

void CreateSurface(Graphics graphics)
{
    VkResult result = glfwCreateWindowSurface(graphics->vkInstance, graphics->window->pHandle, NULL, (VkSurfaceKHR *)&graphics->vkSurface);
    if (result != VK_SUCCESS)
        WriteError(1, "Failed to create window surface");
}

void CreateSwapChain(Graphics graphics)
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(graphics, graphics->vkPhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.pFormats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.pPresentModes, swapChainSupport.presentModeCount);
    VkExtent2D extent = ChooseSwapExtent(graphics->window, swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = graphics->vkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(graphics, graphics->vkPhysicalDevice);
    uint32_t queueFamilyIndices[2] = {indices.graphics, indices.present};

    if (indices.graphics != indices.present)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(graphics->vkDevice, &createInfo, NULL, (VkSwapchainKHR *)&graphics->vkSwapChain);
    if (result != VK_SUCCESS)
        WriteError(1, "Failed to create swap chain");

    vkGetSwapchainImagesKHR(graphics->vkDevice, graphics->vkSwapChain, &imageCount, NULL);
    graphics->vkSwapChainImages = malloc(sizeof(VkImage) * imageCount);
    vkGetSwapchainImagesKHR(graphics->vkDevice, graphics->vkSwapChain, &imageCount, (VkImage *)graphics->vkSwapChainImages);

    graphics->vkSwapChainImageCount = imageCount;
    graphics->vkSwapChainImageFormat = surfaceFormat.format;
    graphics->vkSwapChainImageWidth = extent.width;
    graphics->vkSwapChainImageHeight = extent.height;

    free(swapChainSupport.pFormats);
    free(swapChainSupport.pPresentModes);
}

void CreateImageViews(Graphics graphics)
{
    graphics->vkSwapChainImageViews = malloc(sizeof(VkImageView) * graphics->vkSwapChainImageCount);

    int i;
    for (i = 0; i < graphics->vkSwapChainImageCount; i++)
    {
        VkImageViewCreateInfo createInfo = { 0 };
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = graphics->vkSwapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = graphics->vkSwapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(graphics->vkDevice, &createInfo, NULL, (VkImageView *)&graphics->vkSwapChainImageViews[i]);
        if (result != VK_SUCCESS)
            WriteError(1, "failed to create image views");
    }
}

void CreateRenderPass(Graphics graphics)
{
    VkAttachmentDescription colorAttachment = { 0 };
    colorAttachment.format = graphics->vkSwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkFormat depthFormats[1] = { VK_FORMAT_D32_SFLOAT };
    VkAttachmentDescription depthAttachment = { 0 };
    depthAttachment.format = FindSupportedFormat(graphics, 1, depthFormats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = { 0 };
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = { 0 };
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = { 0 };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = { 0 };
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo = { 0 };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult result = vkCreateRenderPass(graphics->vkDevice, &renderPassInfo, NULL, (VkRenderPass *)&graphics->vkRenderPass);
    if (result != VK_SUCCESS)
        WriteError(1, "Failed to create render pass");

}

void CreateFramebuffers(Graphics graphics)
{
    graphics->vkSwapChainFramebuffers = malloc(sizeof(VkFramebuffer) * graphics->vkSwapChainImageCount);

    int i;
    for (i = 0; i < graphics->vkSwapChainImageCount; i++)
    {
        VkImageView attachments[2] = { graphics->vkSwapChainImageViews[i], graphics->vkDepthImageView };

        VkFramebufferCreateInfo framebufferInfo = { 0 };
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = graphics->vkRenderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = graphics->vkSwapChainImageWidth;
        framebufferInfo.height = graphics->vkSwapChainImageHeight;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(graphics->vkDevice, &framebufferInfo, NULL, (VkFramebuffer *)&graphics->vkSwapChainFramebuffers[i]);
        if (result != VK_SUCCESS)
            WriteError(1, "Failed to create framebuffers");
    }
}

void CreateCommandPool(Graphics graphics)
{
    QueueFamilyIndices indices = FindQueueFamilies(graphics, graphics->vkPhysicalDevice);

    VkCommandPoolCreateInfo poolInfo = { 0 };
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphics;

    VkResult result = vkCreateCommandPool(graphics->vkDevice, &poolInfo, NULL, (VkCommandPool *)&graphics->vkCommandPool);
    if (result != VK_SUCCESS)
        WriteError(1, "Failed to create command pool");
}

void CreateCommandBuffer(Graphics graphics)
{
    VkCommandBufferAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = graphics->vkCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(graphics->vkDevice, &allocInfo, (VkCommandBuffer *)&graphics->vkCommandBuffer);
    if (result != VK_SUCCESS)
        WriteError(1, "Failed to allocate command buffers");
}

void CreateSyncObjects(Graphics graphics)
{
    VkSemaphoreCreateInfo semaphoreInfo = { 0 };
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = { 0 };
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult result1, result2, result3;
    result1 = vkCreateSemaphore(graphics->vkDevice, &semaphoreInfo, NULL, (VkSemaphore *)&graphics->vkImageAvailableSemaphore);
    result2 = vkCreateSemaphore(graphics->vkDevice, &semaphoreInfo, NULL, (VkSemaphore *)&graphics->vkRenderFinishedSemaphore);
    result3 = vkCreateFence(graphics->vkDevice, &fenceInfo, NULL, (VkFence *)&graphics->vkInFlightFence);

    if (result1 != VK_SUCCESS || result2 != VK_SUCCESS || result3 != VK_SUCCESS)
        WriteError(1, "Failed to create semaphores and fences");
}

void RecreateSwapChain(Graphics graphics)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(graphics->window->pHandle, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(graphics->window->pHandle, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(graphics->vkDevice);

    CleanupSwapChain(graphics);

    CreateSwapChain(graphics);
    CreateImageViews(graphics);
    CreateDepthResources(graphics);
    CreateFramebuffers(graphics);
}

void CleanupSwapChain(Graphics graphics)
{
    int i;

    vkDestroyImageView(graphics->vkDevice, graphics->vkDepthImageView, NULL);
    vkDestroyImage(graphics->vkDevice, graphics->vkDepthImage, NULL);
    vkFreeMemory(graphics->vkDevice, graphics->vkDepthImageMemory, NULL);

    for (i = 0; i < graphics->vkSwapChainImageCount; i++)
        vkDestroyFramebuffer(graphics->vkDevice, graphics->vkSwapChainFramebuffers[i], NULL);

    for (i = 0; i < graphics->vkSwapChainImageCount; i++)
        vkDestroyImageView(graphics->vkDevice, graphics->vkSwapChainImageViews[i], NULL);

    vkDestroySwapchainKHR(graphics->vkDevice, graphics->vkSwapChain, NULL);

    free(graphics->vkSwapChainImageViews);
    free(graphics->vkSwapChainFramebuffers);
    free(graphics->vkSwapChainImages);
}

void CreateDepthResources(Graphics graphics)
{
    /*
    {
        VkImageCreateInfo imageInfo = { 0 };
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = graphics->vkSwapChainImageWidth;
        imageInfo.extent.height = graphics->vkSwapChainImageHeight;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateImage(graphics->vkDevice, &imageInfo, NULL, (VkImage *)&graphics->vkDepthImage);

        if (result != VK_SUCCESS)
        {
            WriteError(1, "Failed to create depth image");
            return;
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(graphics->vkDevice, graphics->vkDepthImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo = { 0 };
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(graphics, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        result = vkAllocateMemory(graphics->vkDevice, &allocInfo, NULL, (VkDeviceMemory *)&graphics->vkDepthImageMemory);

        if (result != VK_SUCCESS)
        {
            WriteError(1, "Failed to allocate depth image memory");
            return;
        }

        vkBindImageMemory(graphics->vkDevice, graphics->vkDepthImage, graphics->vkDepthImageMemory, 0);

        VkImageViewCreateInfo viewInfo = { 0 };
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = graphics->vkDepthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(graphics->vkDevice, &viewInfo, NULL, (VkImageView *)&graphics->vkDepthImageView);

        if (result != VK_SUCCESS)
        {
            WriteError(1, "Failed to create depth image view");
            return;
        }
    }
    */

    Texture depthTexture;

    TextureDepthCreateInfo depthInfo = { 0 };
    depthInfo.width = graphics->vkSwapChainImageWidth;
    depthInfo.height = graphics->vkSwapChainImageHeight;

    if (!TextureCreateDepth(graphics, &depthInfo, &depthTexture))
    {
        WriteError(1, "Failed to create depth attachment");
        return;
    }

    graphics->vkDepthImage = depthTexture->vkImage;
    graphics->vkDepthImageMemory = depthTexture->vkImageMemory;
    graphics->vkDepthImageView = depthTexture->vkImageView;

    free(depthTexture);
}

void CreateDescriptorPool(Graphics graphics)
{
    VkDescriptorPoolSize poolSizes[2];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 2;

    VkDescriptorPoolCreateInfo poolInfo = { 0 };
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 2;

    VkResult result = vkCreateDescriptorPool(graphics->vkDevice, &poolInfo, NULL, (VkDescriptorPool *)&graphics->vkDescriptorPool);

    if (result != VK_SUCCESS)
    {
        WriteError(1, "Failed to create descriptor pool");
    }
}

int RateDeviceSuitability(Graphics graphics, VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices indices = FindQueueFamilies(graphics, device);

    if (!indices.hasGraphics || !indices.hasPresent)
        return 0;

    if (!CheckDeviceExtensionSuitability(device))
        return 0;

    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(graphics, device);
    if (!swapChainSupport.formatCount || !swapChainSupport.presentModeCount || !deviceFeatures.samplerAnisotropy || !deviceFeatures.tessellationShader)
        return 0;

    int score = 10;

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 100;

    return score;
}

bool CheckDeviceExtensionSuitability(VkPhysicalDevice device)
{
    unsigned int extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties *availableExtensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    unsigned int requiredExtensionCount = RequiredDeviceExtensionCount;

    int i, j;
    for (i = 0; i < RequiredDeviceExtensionCount; i++)
    {
        for (j = 0; j < extensionCount; j++)
        {
            if (strcmp(RequiredDeviceExtensions[i], availableExtensions[j].extensionName) == 0)
                requiredExtensionCount--;
        }
    }

    return !requiredExtensionCount;
}

SwapChainSupportDetails QuerySwapChainSupport(Graphics graphics, VkPhysicalDevice device)
{
    SwapChainSupportDetails details = { 0 };

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, graphics->vkSurface, &details.capabilities);

    unsigned int formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, graphics->vkSurface, &formatCount, NULL);

    details.formatCount = formatCount;
    details.pFormats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, graphics->vkSurface, &formatCount, details.pFormats);

    unsigned int presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, graphics->vkSurface, &presentModeCount, NULL);

    details.presentModeCount = presentModeCount;
    details.pPresentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, graphics->vkSurface, &presentModeCount, details.pPresentModes);

    return details;
}

QueueFamilyIndices FindQueueFamilies(Graphics graphics, VkPhysicalDevice device)
{
    QueueFamilyIndices indices = { 0 };

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties *queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    int i;
    for (i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, graphics->vkSurface, &presentSupport);

            if (presentSupport)
            {
                indices.present = i;
                indices.hasPresent = true;
            }

            indices.graphics = i;
            indices.hasGraphics = true;

            if (indices.hasGraphics && indices.hasPresent)
                break;
        }
    }

    free(queueFamilies);
    return indices;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(VkSurfaceFormatKHR *pAvailableFormats, unsigned int availableFormatCount)
{
    int i;
    for (i = 0; i < availableFormatCount; i++)
    {
        VkSurfaceFormatKHR surfaceFormat = pAvailableFormats[i];
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return surfaceFormat;
    }

    return pAvailableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(VkPresentModeKHR *pAvailableModes, unsigned int availableModeCount)
{
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

VkExtent2D ChooseSwapExtent(Window window, VkSurfaceCapabilitiesKHR capabilities)
{
    if (capabilities.currentExtent.width != UINT_MAX)
        return capabilities.currentExtent;

    int width, height;
    glfwGetFramebufferSize(window->pHandle, &width, &height);

    VkExtent2D actualExtent = { 0 };
    actualExtent.width = (unsigned int)width;
    actualExtent.height = (unsigned int)height;

    actualExtent.width = CLAMP(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = CLAMP(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func)
        func(instance, debugMessenger, pAllocator);
}

const char **GetRequiredExtensions(GraphicsCreateInfo *pCreateInfo, unsigned int *pExtensionCount)
{
    unsigned int glfwExtensionCount = 0;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    *pExtensionCount = glfwExtensionCount;

    if (pCreateInfo->debug)
        *pExtensionCount += 1;

#ifdef __APPLE__
    *pExtensionCount += 2;
#endif

    const char **extensions = malloc(sizeof(const char *) * (*pExtensionCount));
    memcpy(extensions, glfwExtensions, sizeof(const char *) * glfwExtensionCount);

    unsigned int i = glfwExtensionCount;

    if (pCreateInfo->debug)
    {
        extensions[i] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        i++;
    }

#ifdef __APPLE__
    extensions[i] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    extensions[i + 1] = "VK_KHR_get_physical_device_properties2";
    i += 2;
#endif

    return extensions;
}

VkBool32 DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
                       const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        return VK_FALSE;

    WriteInfo(pCallbackData->pMessage);

    return VK_TRUE;
}

VkFormat FindSupportedFormat(Graphics graphics, unsigned int formatCount, VkFormat *pFormats, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    int i;
    for (i = 0; i < formatCount; i++)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(graphics->vkPhysicalDevice, pFormats[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return pFormats[i];
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return pFormats[i];
    }

    WriteError(1, "Failed to find supported formats");
    return 0;
}