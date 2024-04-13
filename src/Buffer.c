#include "ShlibVK/Buffer.h"
#include "ShlibVK/Graphics.h"
#include "ShlibVK/Utils.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>

unsigned int BufferFindMemoryType(Graphics graphics, unsigned int typeFilter, VkMemoryPropertyFlags properties);

bool BufferCreate(Graphics graphics, BufferCreateInfo *pCreateInfo, Buffer *pBuffer)
{
    Buffer buffer = malloc(sizeof(struct sBuffer));
    buffer->pMappedData = NULL;
    buffer->size = pCreateInfo->size;

    VkBufferCreateInfo bufferInfo = { 0 };
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = pCreateInfo->size;
    bufferInfo.usage = pCreateInfo->usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(graphics->vkDevice, &bufferInfo, NULL, (VkBuffer *)&buffer->vkBuffer);
    if (result != VK_SUCCESS)
    {
        WriteWarning("Failed to create buffer");
        return false;
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(graphics->vkDevice, buffer->vkBuffer, &memReqs);

    VkMemoryPropertyFlags flags = (pCreateInfo->local) ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = BufferFindMemoryType(graphics, memReqs.memoryTypeBits, flags);

    result = vkAllocateMemory(graphics->vkDevice, &allocInfo, NULL, (VkDeviceMemory *)&buffer->vkDeviceMemory);
    if (result != VK_SUCCESS)
    {
        WriteWarning("Failed to allocate vertex buffer memory");
        return false;
    }

    vkBindBufferMemory(graphics->vkDevice, buffer->vkBuffer, buffer->vkDeviceMemory, 0);

    if (pCreateInfo->persistent)
        vkMapMemory(graphics->vkDevice, buffer->vkDeviceMemory, 0, pCreateInfo->size, 0, &buffer->pMappedData);

    *pBuffer = buffer;
    return true;
}

void BufferDestroy(Graphics graphics, Buffer buffer)
{
    vkDeviceWaitIdle(graphics->vkDevice);

    vkDestroyBuffer(graphics->vkDevice, buffer->vkBuffer, NULL);
    vkFreeMemory(graphics->vkDevice, buffer->vkDeviceMemory, NULL);
    free(buffer);
}

void BufferSetData(Graphics graphics, Buffer buffer, void *pData, unsigned int size, unsigned int offset)
{
    void *data;
    vkMapMemory(graphics->vkDevice, buffer->vkDeviceMemory, offset, size, 0, &data);
    memcpy(data, pData, size);
    vkUnmapMemory(graphics->vkDevice, buffer->vkDeviceMemory);
}

void *BufferGetData(Graphics graphics, Buffer buffer, unsigned int size, unsigned int offset)
{
    void *data;
    void *userData = malloc(size);
    vkMapMemory(graphics->vkDevice, buffer->vkDeviceMemory, offset, size, 0, &data);
    memcpy(userData, data, size);
    vkUnmapMemory(graphics->vkDevice, buffer->vkDeviceMemory);

    return userData;
}

void BufferCopy(Graphics graphics, Buffer src, Buffer dst, unsigned int size)
{
    VkCommandBuffer commandBuffer = GraphicsBeginSingleUseCommand(graphics);

    VkBufferCopy copyRegion = { 0 };
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, src->vkBuffer, dst->vkBuffer, 1, &copyRegion);

    GraphicsEndSingleUseCommand(graphics, commandBuffer);
}

void BufferCopyToTexture(Graphics graphics, Buffer src, Texture dst)
{
    VkCommandBuffer commandBuffer = GraphicsBeginSingleUseCommand(graphics);

    VkBufferImageCopy region = { 0 };
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = (VkOffset3D){ 0, 0, 0 };
    region.imageExtent = (VkExtent3D){ dst->width, dst->height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, src->vkBuffer, dst->vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    GraphicsEndSingleUseCommand(graphics, commandBuffer);
}

unsigned int BufferFindMemoryType(Graphics graphics, unsigned int typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(graphics->vkPhysicalDevice, &memProperties);

    for (unsigned int i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    WriteWarning("Failed to find suitable memory type");
    return 0;
}