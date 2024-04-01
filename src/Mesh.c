#include "ShlibVK/Mesh.h"
#include "ShlibVK/Utils.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>

unsigned int FindMemoryType(Mesh mesh, unsigned int typeFilter, VkMemoryPropertyFlags properties);

bool MeshCreate(MeshCreateInfo *pCreateInfo, Mesh *pMesh)
{
    Mesh mesh = malloc(sizeof(struct sMesh));
    mesh->graphics = pCreateInfo->graphics;
    mesh->vertexCount = pCreateInfo->vertexCount;

    VkBufferCreateInfo bufferInfo = { 0 };
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = pCreateInfo->vertexCount * pCreateInfo->stride;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(pCreateInfo->graphics->vkDevice, &bufferInfo, NULL, (VkBuffer *)&mesh->vkBuffer);

    if (result != VK_SUCCESS)
        return false;

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(mesh->graphics->vkDevice, mesh->vkBuffer, &memReqs);

    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(mesh, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    result = vkAllocateMemory(mesh->graphics->vkDevice, &allocInfo, NULL, (VkDeviceMemory *)&mesh->vkDeviceMemory);
    if (result != VK_SUCCESS)
    {
        WriteWarning("Failed to allocate vertex buffer memory");
        return false;
    }

    vkBindBufferMemory(mesh->graphics->vkDevice, mesh->vkBuffer, mesh->vkDeviceMemory, 0);

    void *data;
    vkMapMemory(mesh->graphics->vkDevice, mesh->vkDeviceMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, pCreateInfo->pVertices, bufferInfo.size);
    vkUnmapMemory(mesh->graphics->vkDevice, mesh->vkDeviceMemory);

    *pMesh = mesh;
    return true;
}

void MeshDestroy(Mesh mesh)
{
    vkDeviceWaitIdle(mesh->graphics->vkDevice);

    vkDestroyBuffer(mesh->graphics->vkDevice, mesh->vkBuffer, NULL);
    vkFreeMemory(mesh->graphics->vkDevice, mesh->vkDeviceMemory, NULL);
    free(mesh);
}

void MeshDraw(Mesh mesh)
{
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(mesh->graphics->vkCommandBuffer, 0, 1, (VkBuffer *)&mesh->vkBuffer, &offset);

    vkCmdDraw(mesh->graphics->vkCommandBuffer, mesh->vertexCount, 1, 0, 0);
}

unsigned int FindMemoryType(Mesh mesh, unsigned int typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(mesh->graphics->vkPhysicalDevice, &memProperties);

    for (unsigned int i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    WriteWarning("Failed to find suitable memory type");
    return 0;
}