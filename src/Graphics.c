#include "ShlibVK/Graphics.h"
#include "ShlibVK/Utils.h"
#include "ShlibVK/Window.h"


#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <stdlib.h>
#include <string.h>

const char *ValidationLayers[] = { "VK_LAYER_KHRONOS_validation" };
const unsigned int RequiredDeviceExtensionCount = 1;
const char *RequiredDeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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

void CreateInstance(GraphicsCreateInfo *pCreateInfo, Graphics graphics);
void CreateDebugMessenger(Graphics graphics);
void PickPhysicalDevice(GraphicsCreateInfo *pCreateInfo, Graphics graphics);
void CreateLogicalDevice(GraphicsCreateInfo *pCreateInfo, Graphics graphics);
void CreateSurface(Graphics graphics, Window window);
void CreateSwapChain(Graphics graphics, Window window);
void CreateImageViews(Graphics graphics);
void CreateGraphicsPipeline(Graphics graphics);

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

bool GraphicsCreate(GraphicsCreateInfo *pCreateInfo, Graphics *pGraphics)
{
    Graphics graphics = malloc(sizeof(struct sGraphics));

    CreateInstance(pCreateInfo, graphics);
    if (pCreateInfo->debug)
        CreateDebugMessenger(graphics);
    CreateSurface(graphics, pCreateInfo->window);
    PickPhysicalDevice(pCreateInfo, graphics);
    CreateLogicalDevice(pCreateInfo, graphics);
    CreateSwapChain(graphics, pCreateInfo->window);
    CreateImageViews(graphics);
    CreateGraphicsPipeline(graphics);

    *pGraphics = graphics;
    return true;
}

void GraphicsDestroy(Graphics graphics)
{
    if (!graphics)
        return;

    for (int i = 0; i < graphics->vkSwapChainImageCount; i++)
        vkDestroyImageView(graphics->vkDevice, graphics->vkSwapChainImageViews[i], NULL);
    free(graphics->vkSwapChainImageViews);
    free(graphics->vkSwapChainImages);

    vkDestroySwapchainKHR(graphics->vkDevice, graphics->vkSwapChain, NULL);
    vkDestroyDevice(graphics->vkDevice, NULL);
    vkDestroySurfaceKHR(graphics->vkInstance, graphics->vkSurface, NULL);
    if (graphics->vkDebugMessenger)
        DestroyDebugUtilsMessengerEXT(graphics->vkInstance, graphics->vkDebugMessenger, NULL);
    vkDestroyInstance(graphics->vkInstance, NULL);
    free(graphics);
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

    for (int i = 1; i < deviceCount; i++)
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

    for (int i = 0; i < queueFamilyCount; i++)
    {
        bool unique = true;

        for (int j = 0; j < uniqueQueueFamilyCount; j++)
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

    for (int i = 0; i < uniqueQueueFamilyCount; i++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = { 0 };
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = { 0 };

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

    vkGetDeviceQueue(graphics->vkDevice, indices.graphics, 0, (VkQueue *)&graphics->vkGraphicsQueue);
    vkGetDeviceQueue(graphics->vkDevice, indices.present, 0, (VkQueue *)&graphics->vkPresentQueue);
}

void CreateSurface(Graphics graphics, Window window)
{
    VkResult result = glfwCreateWindowSurface(graphics->vkInstance, window->pHandle, NULL, (VkSurfaceKHR *)&graphics->vkSurface);
    if (result != VK_SUCCESS)
        WriteError(1, "Failed to create window surface");
}

void CreateSwapChain(Graphics graphics, Window window)
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(graphics, graphics->vkPhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.pFormats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.pPresentModes, swapChainSupport.presentModeCount);
    VkExtent2D extent = ChooseSwapExtent(window, swapChainSupport.capabilities);

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
}

void CreateImageViews(Graphics graphics)
{
    graphics->vkSwapChainImageViews = malloc(sizeof(VkImageView) * graphics->vkSwapChainImageCount);

    for (int i = 0; i < graphics->vkSwapChainImageCount; i++)
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

void CreateGraphicsPipeline(Graphics graphics)
{

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
    if (!swapChainSupport.formatCount || !swapChainSupport.presentModeCount)
        return 0;

    int score = 0;

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

    for (int i = 0; i < RequiredDeviceExtensionCount; i++)
    {
        for (int j = 0; j < extensionCount; j++)
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

    for (int i = 0; i < queueFamilyCount; i++)
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

    return indices;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(VkSurfaceFormatKHR *pAvailableFormats, unsigned int availableFormatCount)
{
    for (int i = 0; i < availableFormatCount; i++)
    {
        VkSurfaceFormatKHR surfaceFormat = pAvailableFormats[i];
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return surfaceFormat;
    }

    return pAvailableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(VkPresentModeKHR *pAvailableModes, unsigned int availableModeCount)
{
    return VK_PRESENT_MODE_FIFO_KHR;
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

    const char **extensions = malloc(sizeof(const char *) * (*pExtensionCount));
    memcpy(extensions, glfwExtensions, sizeof(const char *) * glfwExtensionCount);

    unsigned int i = glfwExtensionCount;

    if (pCreateInfo->debug)
    {
        extensions[i] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        i++;
    }

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
