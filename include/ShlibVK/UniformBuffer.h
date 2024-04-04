#ifndef SHLIBVK_UNIFORMBUFFER_H
#define SHLIBVK_UNIFORMBUFFER_H

#include "Buffer.h"

struct sUniformBufferCreateInfo
{
    unsigned int size;
    unsigned int binding;
};

struct sUniformBuffer
{
    Buffer buffer;
    unsigned int binding;
};

typedef struct sUniformBufferCreateInfo UniformBufferCreateInfo;
typedef struct sUniformBuffer *UniformBuffer;

bool UniformBufferCreate(Graphics graphics, UniformBufferCreateInfo *pCreateInfo, UniformBuffer *pUniformBuffer);
void UniformBufferDestroy(Graphics graphics, UniformBuffer uniformBuffer);

void UniformBufferSetData(UniformBuffer uniformBuffer, void *pData, unsigned int size);

#endif //SHLIBVK_UNIFORMBUFFER_H
