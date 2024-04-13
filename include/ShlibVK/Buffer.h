#ifndef SHLIBVK_BUFFER_H
#define SHLIBVK_BUFFER_H

#include "Graphics.h"
#include "Texture.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum eBufferUsage {
    USAGE_TRANSFER_SRC = 0x01,
    USAGE_TRANSFER_DST = 0x02,
    USAGE_UNIFORM_BUFFER = 0x10,
    USAGE_STORAGE_BUFFER = 0x20,
    USAGE_INDEX_BUFFER = 0x40,
    USAGE_VERTEX_BUFFER = 0x80,
} BufferUsage;

struct sBufferCreateInfo {
    unsigned int size;
    BufferUsage usage;
    bool local;
    bool persistent;
};

struct sBuffer {
    unsigned int size;

    void *vkBuffer;
    void *vkDeviceMemory;
    void *pMappedData;
};

typedef struct sBufferCreateInfo BufferCreateInfo;
typedef struct sBuffer *Buffer;

bool BufferCreate(Graphics graphics, BufferCreateInfo *pCreateInfo, Buffer *pBuffer);

void BufferDestroy(Graphics graphics, Buffer buffer);

void BufferSetData(Graphics graphics, Buffer buffer, void *pData, unsigned int size, unsigned int offset);

void *BufferGetData(Graphics graphics, Buffer buffer, unsigned int size, unsigned int offset);

void BufferCopy(Graphics graphics, Buffer src, Buffer dst, unsigned int size);

void BufferCopyToTexture(Graphics graphics, Buffer src, Texture dst);

#ifdef __cplusplus
}
#endif

#endif //SHLIBVK_BUFFER_H
