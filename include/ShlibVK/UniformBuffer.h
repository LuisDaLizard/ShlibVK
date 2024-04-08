#ifndef SHLIBVK_UNIFORMBUFFER_H
#define SHLIBVK_UNIFORMBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Buffer.h"

struct sUniformBufferCreateInfo {
    unsigned int size;
    unsigned int binding;
};

struct sUniformBuffer {
    Buffer buffer;
    unsigned int binding;
};

typedef struct sUniformBufferCreateInfo UniformBufferCreateInfo;
typedef struct sUniformBuffer *UniformBuffer;

bool UniformBufferCreate(Graphics graphics, UniformBufferCreateInfo *pCreateInfo, UniformBuffer *pUniformBuffer);

void UniformBufferDestroy(Graphics graphics, UniformBuffer uniformBuffer);

void UniformBufferSetData(UniformBuffer uniformBuffer, void *pData, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif //SHLIBVK_UNIFORMBUFFER_H
