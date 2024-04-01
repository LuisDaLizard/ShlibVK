#include "ShlibVK/Mesh.h"
#include "ShlibVK/Utils.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>

bool MeshCreate(Graphics graphics, MeshCreateInfo *pCreateInfo, Mesh *pMesh)
{
    Mesh mesh = malloc(sizeof(struct sMesh));
    mesh->vertexCount = pCreateInfo->vertexCount;

    unsigned int size = mesh->vertexCount * pCreateInfo->stride;

    Buffer stagingBuffer;

    BufferCreateInfo stagingInfo = { 0 };
    stagingInfo.local = false;
    stagingInfo.size = size;
    stagingInfo.usage = USAGE_TRANSFER_SRC;

    if (!BufferCreate(graphics, &stagingInfo, &stagingBuffer))
    {
        WriteWarning("Unable to create staging buffer");
        return false;
    }

    BufferSetData(graphics, stagingBuffer, pCreateInfo->pVertices, size, 0);

    BufferCreateInfo vertexInfo = { 0 };
    vertexInfo.local = true;
    vertexInfo.size = size;
    vertexInfo.usage = USAGE_TRANSFER_DST | USAGE_VERTEX_BUFFER;

    if (!BufferCreate(graphics, &vertexInfo, &mesh->vertexBuffer))
    {
        WriteWarning("Unable to create vertex buffer");
        return false;
    }

    BufferCopy(graphics, stagingBuffer, mesh->vertexBuffer, size);

    BufferDestroy(graphics, stagingBuffer);

    *pMesh = mesh;
    return true;
}

void MeshDestroy(Graphics graphics, Mesh mesh)
{
    vkDeviceWaitIdle(graphics->vkDevice);

    BufferDestroy(graphics, mesh->vertexBuffer);

    free(mesh);
}

void MeshDraw(Graphics graphics, Mesh mesh)
{
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(graphics->vkCommandBuffer, 0, 1, (VkBuffer *)&mesh->vertexBuffer->vkBuffer, &offset);

    vkCmdDraw(graphics->vkCommandBuffer, mesh->vertexCount, 1, 0, 0);
}