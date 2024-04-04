#include "ShlibVK/UniformBuffer.h"
#include "ShlibVK/Utils.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>

bool UniformBufferCreate(Graphics graphics, UniformBufferCreateInfo *pCreateInfo, UniformBuffer *pUniformBuffer)
{
    VkDeviceSize size = pCreateInfo->size;

    BufferCreateInfo bufferInfo = { 0 };
    bufferInfo.size = size;
    bufferInfo.usage = USAGE_UNIFORM_BUFFER;
    bufferInfo.local = false;
    bufferInfo.persistent = true;

    UniformBuffer uniformBuffer = malloc(sizeof(struct sUniformBuffer));
    uniformBuffer->binding = pCreateInfo->binding;
    if (!BufferCreate(graphics, &bufferInfo, &uniformBuffer->buffer))
    {
        WriteWarning("Failed to create uniform buffer");
        return false;
    }

    *pUniformBuffer = uniformBuffer;
    return true;
}

void UniformBufferDestroy(Graphics graphics, UniformBuffer uniformBuffer)
{
    vkDeviceWaitIdle(graphics->vkDevice);

    BufferDestroy(graphics, uniformBuffer->buffer);
    free(uniformBuffer);
}

void UniformBufferSetData(UniformBuffer uniformBuffer, void *pData, unsigned int size)
{
    memcpy(uniformBuffer->buffer->pMappedData, pData, size);
}