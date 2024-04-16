#include "ShlibVK/Mesh.h"
#include "ShlibVK/Utils.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>

bool MeshCreate(Graphics graphics, MeshCreateInfo *pCreateInfo, Mesh *pMesh)
{
    Mesh mesh = malloc(sizeof(struct sMesh));
    mesh->vertexCount = pCreateInfo->vertexCount;
    mesh->bufferCount = pCreateInfo->bufferCount;
    mesh->pVertexBuffers = malloc(sizeof(Buffer) * mesh->bufferCount);

    int i;
    for (i = 0; i < mesh->bufferCount; i++)
    {
        unsigned int stride = pCreateInfo->strides[i];
        unsigned int size = mesh->vertexCount * stride;

        Buffer stagingBuffer;

        BufferCreateInfo stagingInfo = { 0 };
        stagingInfo.local = false;
        stagingInfo.size = size;
        stagingInfo.usage = USAGE_TRANSFER_SRC;

        if (!BufferCreate(graphics, &stagingInfo, &stagingBuffer))
        {
            WriteWarning("Failed to create staging buffer");
            return false;
        }

        BufferSetData(graphics, stagingBuffer, pCreateInfo->ppData[i], size, 0);

        BufferCreateInfo vertexInfo = { 0 };
        vertexInfo.local = true;
        vertexInfo.size = size;
        vertexInfo.usage = USAGE_TRANSFER_DST | USAGE_VERTEX_BUFFER | USAGE_STORAGE_BUFFER;

        if (!BufferCreate(graphics, &vertexInfo, &mesh->pVertexBuffers[i]))
        {
            WriteWarning("Unable to create vertex buffer");
            return false;
        }

        BufferCopy(graphics, stagingBuffer, mesh->pVertexBuffers[i], size);

        BufferDestroy(graphics, stagingBuffer);
    }

    *pMesh = mesh;
    return true;
}

void MeshDestroy(Graphics graphics, Mesh mesh)
{
    vkDeviceWaitIdle(graphics->vkDevice);

    int i;
    for (i = 0; i < mesh->bufferCount; i++)
        BufferDestroy(graphics, mesh->pVertexBuffers[i]);

    free(mesh->pVertexBuffers);
    free(mesh);
}

Buffer MeshGetBuffer(Mesh mesh, unsigned int index)
{
    if (index >= mesh->bufferCount)
        return NULL;
    return mesh->pVertexBuffers[index];
}

StorageBuffer MeshGetStorageBuffer(Mesh mesh, unsigned int index)
{
    if (index >= mesh->bufferCount)
        return NULL;
    return (StorageBuffer)mesh->pVertexBuffers[index];
}

void MeshDraw(Graphics graphics, Mesh mesh)
{
    VkBuffer *buffers = malloc(sizeof(VkBuffer) * mesh->bufferCount);
    VkDeviceSize *offsets = calloc(mesh->bufferCount, sizeof(VkDeviceSize));

    int i;
    for (i = 0; i < mesh->bufferCount; i++)
        buffers[i] = mesh->pVertexBuffers[i]->vkBuffer;

    vkCmdBindVertexBuffers(graphics->vkCommandBuffer, 0, mesh->bufferCount, buffers, offsets);

    vkCmdDraw(graphics->vkCommandBuffer, mesh->vertexCount, 1, 0, 0);

    free(buffers);
}