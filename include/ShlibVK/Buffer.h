#ifndef SHLIBVK_BUFFER_H
#define SHLIBVK_BUFFER_H

#include "Graphics.h"
#include <stdbool.h>

typedef enum eBufferUsage
{
    USAGE_TRANSFER_SRC = 0x01,
    USAGE_TRANSFER_DST = 0x02,
    USAGE_INDEX_BUFFER = 0x40,
    USAGE_VERTEX_BUFFER = 0x80,
} BufferUsage;

struct sBufferCreateInfo
{
    unsigned int size;
    BufferUsage usage;
    bool local;
};

struct sBuffer
{
    unsigned int size;

    void *vkBuffer;
    void *vkDeviceMemory;
};

typedef struct sBufferCreateInfo BufferCreateInfo;
typedef struct sBuffer *Buffer;

bool BufferCreate(Graphics graphics, BufferCreateInfo *pCreateInfo, Buffer *pBuffer);
void BufferDestroy(Graphics graphics, Buffer buffer);

void BufferSetData(Graphics graphics, Buffer buffer, void *pData, unsigned int size, unsigned int offset);
void BufferCopy(Graphics graphics, Buffer src, Buffer dst, unsigned int size);

#endif //SHLIBVK_BUFFER_H
