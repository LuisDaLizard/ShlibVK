#include "ShlibVK/StorageBuffer.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>

bool StorageBufferCreate(Graphics graphics, StorageBufferCreateInfo *pCreateInfo, StorageBuffer *pStorageBuffer)
{
    StorageBuffer storageBuffer = malloc(sizeof(struct sStorageBuffer));

    BufferCreateInfo bufferInfo = { 0 };
    bufferInfo.size = pCreateInfo->size;
    bufferInfo.usage = USAGE_STORAGE_BUFFER | USAGE_VERTEX_BUFFER | USAGE_TRANSFER_DST;
    bufferInfo.persistent = false;
    bufferInfo.local = false;

    if (!BufferCreate(graphics, &bufferInfo, &storageBuffer->buffer))
        return false;

    *pStorageBuffer = storageBuffer;
    return true;
}

void StorageBufferDestroy(Graphics graphics, StorageBuffer storageBuffer)
{
    vkDeviceWaitIdle(graphics->vkDevice);

    BufferDestroy(graphics, storageBuffer->buffer);
    free(storageBuffer);
}

void StorageBufferSetData(Graphics graphics, StorageBuffer storageBuffer, unsigned int size, void *pData)
{
    BufferSetData(graphics, storageBuffer->buffer, pData, size, 0);
}