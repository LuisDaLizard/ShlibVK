#ifndef SHLIBVK_STORAGEBUFFER_H
#define SHLIBVK_STORAGEBUFFER_H

#include "Buffer.h"

struct sStorageBufferCreateInfo
{
    unsigned int size;
};

struct sStorageBuffer
{
    Buffer buffer;
};

typedef struct sStorageBufferCreateInfo StorageBufferCreateInfo;
typedef struct sStorageBuffer *StorageBuffer;

bool StorageBufferCreate(Graphics graphics, StorageBufferCreateInfo *pCreateInfo, StorageBuffer *pStorageBuffer);
void StorageBufferDestroy(Graphics graphics, StorageBuffer storageBuffer);

void StorageBufferSetData(Graphics graphics, StorageBuffer storageBuffer, unsigned int size, void *pData);

#endif //SHLIBVK_STORAGEBUFFER_H
