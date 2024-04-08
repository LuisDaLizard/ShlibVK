#include "ShlibVK/Texture.h"
#include "ShlibVK/Utils.h"
#include "ShlibVK/Buffer.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>

bool CreateTextureImage(Graphics graphics, Texture texture, TextureCreateInfo *pCreateInfo);
bool CreateTextureImageView(Graphics graphics, Texture texture, TextureCreateInfo *pCreateInfo);
bool CreateTextureSampler(Graphics graphics, Texture texture, TextureCreateInfo *pCreateInfo);
unsigned int FindMemoryType(Graphics graphics, unsigned int typeFilter, VkMemoryPropertyFlags properties);
void TransitionImageLayout(Graphics graphics, Texture texture, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

bool TextureCreate(Graphics graphics, TextureCreateInfo *pCreateInfo, Texture *pTexture)
{
    Texture texture = malloc(sizeof(struct sTexture));
    texture->width = pCreateInfo->width;
    texture->height = pCreateInfo->height;

    if (!CreateTextureImage(graphics, texture, pCreateInfo))
    {
        WriteWarning("Failed to create texture image");
        return false;
    }

    if (!CreateTextureImageView(graphics, texture, pCreateInfo))
    {
        WriteWarning("Failed to create texture image view");
        return false;
    }

    if (!CreateTextureSampler(graphics, texture, pCreateInfo))
    {
        WriteWarning("Failed to create texture sampler");
        return false;
    }

    *pTexture = texture;
    return true;
}

void TextureDestroy(Graphics graphics, Texture texture)
{
    vkDeviceWaitIdle(graphics->vkDevice);

    vkDestroySampler(graphics->vkDevice, texture->vkSampler, NULL);
    vkDestroyImageView(graphics->vkDevice, texture->vkImageView, NULL);
    vkDestroyImage(graphics->vkDevice, texture->vkImage, NULL);
    vkFreeMemory(graphics->vkDevice, texture->vkImageMemory, NULL);

    free(texture);
}

bool CreateTextureImage(Graphics graphics, Texture texture, TextureCreateInfo *pCreateInfo)
{
    Buffer stagingBuffer;
    BufferCreateInfo bufferInfo = { 0 };
    bufferInfo.size = texture->width * texture->height * pCreateInfo->channels;
    bufferInfo.local = false;
    bufferInfo.persistent = false;
    bufferInfo.usage = USAGE_TRANSFER_SRC;

    BufferCreate(graphics, &bufferInfo, &stagingBuffer);
    BufferSetData(graphics, stagingBuffer, pCreateInfo->pData, bufferInfo.size, 0);

    switch(pCreateInfo->channels)
    {
        case 3:
            texture->format = VK_FORMAT_R8G8B8_SRGB;
            break;
        case 4:
            texture->format = VK_FORMAT_R8G8B8A8_SRGB;
            break;
        default:
            WriteWarning("Unsupported channel number");
            return false;
    }

    VkImageCreateInfo imageInfo = { 0 };
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = texture->width;
    imageInfo.extent.height = texture->height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = texture->format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VkResult result = vkCreateImage(graphics->vkDevice, &imageInfo, NULL, (VkImage *)&texture->vkImage);

    if (result != VK_SUCCESS)
    {
        WriteWarning("Failed to create image");
        return false;
    }

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(graphics->vkDevice, texture->vkImage, &memReqs);

    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(graphics, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    result = vkAllocateMemory(graphics->vkDevice, &allocInfo, NULL, (VkDeviceMemory *)&texture->vkImageMemory);

    if (result != VK_SUCCESS)
    {
        WriteWarning("Failed to allocate image memory");
        return false;
    }

    vkBindImageMemory(graphics->vkDevice, texture->vkImage, texture->vkImageMemory, 0);

    TransitionImageLayout(graphics, texture, texture->format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    BufferCopyToTexture(graphics, stagingBuffer, texture);

    TransitionImageLayout(graphics, texture, texture->format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    BufferDestroy(graphics, stagingBuffer);

    return true;
}

bool CreateTextureImageView(Graphics graphics, Texture texture, TextureCreateInfo *pCreateInfo)
{
    VkImageViewCreateInfo viewInfo = { 0 };
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture->vkImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = texture->format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(graphics->vkDevice, &viewInfo, NULL, (VkImageView *)&texture->vkImageView);

    if (result != VK_SUCCESS)
        return false;

    return true;
}

bool CreateTextureSampler(Graphics graphics, Texture texture, TextureCreateInfo *pCreateInfo)
{
    VkPhysicalDeviceProperties properties = { 0 };
    vkGetPhysicalDeviceProperties(graphics->vkPhysicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo = { 0 };
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkResult result = vkCreateSampler(graphics->vkDevice, &samplerInfo, NULL, (VkSampler *)&texture->vkSampler);

    if (result != VK_SUCCESS)
    {
        return false;
    }

    return true;
}

unsigned int FindMemoryType(Graphics graphics, unsigned int typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(graphics->vkPhysicalDevice, &memProperties);

    for (unsigned int i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    WriteWarning("Failed to find suitable memory type");
    return 0;
}

void TransitionImageLayout(Graphics graphics, Texture texture, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = GraphicsBeginSingleUseCommand(graphics);

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;
    VkImageMemoryBarrier barrier = { 0 };
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture->vkImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        WriteError(1, "Unsupported layout transition");
        return;
    }

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &barrier );

    GraphicsEndSingleUseCommand(graphics, commandBuffer);
}