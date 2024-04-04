#include "ShlibVK/Buffer.h"
#include "ShlibVK/Graphics.h"
#include "ShlibVK/Utils.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>

unsigned int FindMemoryType(Graphics graphics, unsigned int typeFilter, VkMemoryPropertyFlags properties);

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
    allocInfo.memoryTypeIndex = FindMemoryType(graphics, memReqs.memoryTypeBits, flags);

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

unsigned int FindMemoryType(Graphics graphics, unsigned int typeFilter, VkMemoryPropertyFlags properties)
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