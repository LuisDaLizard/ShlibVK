#include "ShlibVK/Pipeline.h"
#include "ShlibVK/Utils.h"
#include "ShlibVK/Graphics.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>

VkShaderModule CreateShaderModule(Graphics graphics, const unsigned int *pShaderCode, unsigned int shaderCodeSize);
bool CreateDescriptorSetLayout(Graphics graphics, Pipeline pipeline, PipelineCreateInfo *pCreateInfo);
bool CreateDescriptorSets(Graphics graphics, Pipeline pipeline, PipelineCreateInfo *pCreateInfo);
bool CreatePipeline(Graphics graphics, Pipeline pipeline, PipelineCreateInfo *pCreateInfo, VkShaderModule vertShader, VkShaderModule tessCtrl, VkShaderModule tessEval, VkShaderModule fragShader);

bool PipelineCreate(Graphics graphics, PipelineCreateInfo *pCreateInfo, Pipeline *pPipeline)
{
    if (!pCreateInfo->pVertexShaderCode || !pCreateInfo->pFragmentShaderCode)
        return false;

    Pipeline pipeline = malloc(sizeof(struct sPipeline));

    VkShaderModule vertex = CreateShaderModule(graphics, pCreateInfo->pVertexShaderCode, pCreateInfo->vertexShaderSize);
    VkShaderModule fragment = CreateShaderModule(graphics, pCreateInfo->pFragmentShaderCode, pCreateInfo->fragmentShaderSize);
    VkShaderModule tessEval = NULL;
    VkShaderModule tessCtrl = NULL;

    if (pCreateInfo->pTessCtrlCode && pCreateInfo->pTessEvalCode)
    {
        tessCtrl = CreateShaderModule(graphics, pCreateInfo->pTessCtrlCode, pCreateInfo->tessCtrlShaderSize);
        tessEval = CreateShaderModule(graphics, pCreateInfo->pTessEvalCode, pCreateInfo->tessEvalShaderSize);
    }

    if (!CreateDescriptorSetLayout(graphics, pipeline, pCreateInfo))
        return false;

    if (!CreatePipeline(graphics, pipeline, pCreateInfo, vertex, tessCtrl, tessEval, fragment))
        return false;

    if (!CreateDescriptorSets(graphics, pipeline, pCreateInfo))
        return false;

    vkDestroyShaderModule(graphics->vkDevice, vertex, NULL);
    vkDestroyShaderModule(graphics->vkDevice, fragment, NULL);

    if (pCreateInfo->pTessCtrlCode && pCreateInfo->pTessEvalCode)
    {
        vkDestroyShaderModule(graphics->vkDevice, tessCtrl, NULL);
        vkDestroyShaderModule(graphics->vkDevice, tessEval, NULL);
    }

    *pPipeline = pipeline;
    return true;
}

void PipelineDestroy(Graphics graphics, Pipeline pipeline)
{
    vkDeviceWaitIdle(graphics->vkDevice);

    vkDestroyDescriptorSetLayout(graphics->vkDevice, pipeline->vkDescriptorSetLayout, NULL);
    vkDestroyPipeline(graphics->vkDevice, pipeline->vkGraphicsPipeline, NULL);
    vkDestroyPipelineLayout(graphics->vkDevice, pipeline->vkPipelineLayout, NULL);

    free(pipeline);
}

void PipelineBind(Graphics graphics, Pipeline pipeline)
{
    vkCmdBindPipeline(graphics->vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vkGraphicsPipeline);
    vkCmdBindDescriptorSets(graphics->vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vkPipelineLayout, 0, 1, (VkDescriptorSet *)&pipeline->vkDescriptorSet, 0, NULL);
}

VkShaderModule CreateShaderModule(Graphics graphics, const unsigned int *pShaderCode, unsigned int shaderCodeSize)
{
    VkShaderModule shaderModule;

    VkShaderModuleCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCodeSize;
    createInfo.pCode = pShaderCode;

    VkResult result = vkCreateShaderModule(graphics->vkDevice, &createInfo, NULL, &shaderModule);
    if (result != VK_SUCCESS)
    {
        WriteWarning("Failed to create shader module");
        return 0;
    }

    return shaderModule;
}

bool CreateDescriptorSetLayout(Graphics graphics, Pipeline pipeline, PipelineCreateInfo *pCreateInfo)
{
    int i;
    VkDescriptorSetLayoutBinding  *layouts = malloc(sizeof(VkDescriptorSetLayoutBinding) * (pCreateInfo->descriptorCount));
    for (i = 0; i < pCreateInfo->descriptorCount; i++)
    {
        VkDescriptorType type;

        switch (pCreateInfo->pDescriptors[i].type)
        {
            case DESCRIPTOR_TYPE_SAMPLER:
                type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
            case DESCRIPTOR_TYPE_UNIFORM:
                type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            default:
                return false;
        }

        layouts[i].binding = pCreateInfo->pDescriptors[i].location;
        layouts[i].descriptorType = type;
        layouts[i].descriptorCount = pCreateInfo->pDescriptors[i].count;
        layouts[i].stageFlags = pCreateInfo->pDescriptors[i].stage;
        layouts[i].pImmutableSamplers = NULL;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = { 0 };
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = pCreateInfo->descriptorCount;
    layoutInfo.pBindings = layouts;
    VkResult result = vkCreateDescriptorSetLayout(graphics->vkDevice, &layoutInfo, NULL, (VkDescriptorSetLayout *)&pipeline->vkDescriptorSetLayout);

    if (result != VK_SUCCESS)
    {
        WriteWarning("Failed to create descriptor set layout");
        return false;
    }

    free(layouts);

    return true;
}

bool CreateDescriptorSets(Graphics graphics, Pipeline pipeline, PipelineCreateInfo *pCreateInfo)
{
    VkDescriptorSetAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = graphics->vkDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = (VkDescriptorSetLayout *)&pipeline->vkDescriptorSetLayout;

    VkResult result = vkAllocateDescriptorSets(graphics->vkDevice, &allocInfo, (VkDescriptorSet *)&pipeline->vkDescriptorSet);

    if (result != VK_SUCCESS)
    {
        WriteWarning("Failed to create descriptor set!");
        return false;
    }

    int i;
    for (i = 0; i < pCreateInfo->descriptorCount; i++)
    {
        VkWriteDescriptorSet descriptorWrite = { 0 };
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = pipeline->vkDescriptorSet;
        descriptorWrite.dstBinding = pCreateInfo->pDescriptors[i].location;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;

        switch(pCreateInfo->pDescriptors[i].type)
        {
            case DESCRIPTOR_TYPE_UNIFORM:
            {
                VkDescriptorBufferInfo bufferInfo = { 0 };
                bufferInfo.buffer = pCreateInfo->pDescriptors[i].uniform->buffer->vkBuffer;
                bufferInfo.offset = 0;
                bufferInfo.range = pCreateInfo->pDescriptors[i].uniform->buffer->size;

                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.pBufferInfo = &bufferInfo;
                break;
            }
            case DESCRIPTOR_TYPE_SAMPLER:
            {
                VkDescriptorImageInfo imageInfo = { 0 };
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = pCreateInfo->pDescriptors[i].texture->vkImageView;
                imageInfo.sampler = pCreateInfo->pDescriptors[i].texture->vkSampler;

                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.pImageInfo = &imageInfo;
                break;
            }
            default:
                return false;
        }

        vkUpdateDescriptorSets(graphics->vkDevice, 1, &descriptorWrite, 0, NULL);
    }

    return true;
}

bool CreatePipeline(Graphics graphics, Pipeline pipeline, PipelineCreateInfo *pCreateInfo, VkShaderModule vertShader, VkShaderModule tessCtrl, VkShaderModule tessEval, VkShaderModule fragShader)
{
    bool useTesselation = (pCreateInfo->pTessCtrlCode && pCreateInfo->pTessEvalCode);

    unsigned int stageCount = 2, stageIndex = 0;

    if (useTesselation)
    {
        stageCount += 2;
    }

    VkPipelineShaderStageCreateInfo *shaderStages = malloc(sizeof(VkPipelineShaderStageCreateInfo) * stageCount);

    VkPipelineShaderStageCreateInfo vertexStage = { 0 };
    vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStage.module = vertShader;
    vertexStage.pName = "main";

    shaderStages[stageIndex] = vertexStage;
    stageIndex++;

    if (useTesselation)
    {
        VkPipelineShaderStageCreateInfo tessCtrlStage = { 0 };
        tessCtrlStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        tessCtrlStage.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        tessCtrlStage.module = tessCtrl;
        tessCtrlStage.pName = "main";

        VkPipelineShaderStageCreateInfo tessEvalStage = { 0 };
        tessEvalStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        tessEvalStage.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        tessEvalStage.module = tessEval;
        tessEvalStage.pName = "main";

        shaderStages[stageIndex] = tessCtrlStage;
        stageIndex++;

        shaderStages[stageIndex] = tessEvalStage;
        stageIndex++;
    }

    VkPipelineShaderStageCreateInfo fragmentStage = { 0 };
    fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStage.module = fragShader;
    fragmentStage.pName = "main";

    shaderStages[stageIndex] = fragmentStage;
    stageIndex++;

    VkVertexInputBindingDescription bindingDescription = { 0 };
    bindingDescription.binding = 0;
    bindingDescription.stride = pCreateInfo->stride;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription *attributeDescriptions = malloc(sizeof(VkVertexInputAttributeDescription) * pCreateInfo->attributeCount);
    int i;
    for (i = 0; i < pCreateInfo->attributeCount; i++)
    {
        unsigned int format;

        switch(pCreateInfo->pAttributes[i].components)
        {
            case 1:
                format = VK_FORMAT_R32_SFLOAT;
                break;
            case 2:
                format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case 3:
                format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case 4:
                format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            default:
                WriteWarning("Unknown component count");
                return false;
        }

        attributeDescriptions[i].binding = 0;
        attributeDescriptions[i].location = pCreateInfo->pAttributes[i].location;
        attributeDescriptions[i].offset = pCreateInfo->pAttributes[i].offset;
        attributeDescriptions[i].format = format;
    }

    VkPipelineVertexInputStateCreateInfo vertexInput = { 0 };
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &bindingDescription;
    vertexInput.vertexAttributeDescriptionCount = pCreateInfo->attributeCount;
    vertexInput.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { 0 };
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = (VkPrimitiveTopology)pCreateInfo->topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) graphics->vkSwapChainImageWidth;
    viewport.height = (float) graphics->vkSwapChainImageHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = { 0 };
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent.width = graphics->vkSwapChainImageWidth;
    scissor.extent.height = graphics->vkSwapChainImageHeight;

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState = { 0 };
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineViewportStateCreateInfo viewportState = { 0 };
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = { 0 };
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineTessellationStateCreateInfo  tesselation = { 0 };
    tesselation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tesselation.patchControlPoints = pCreateInfo->patchSize;

    VkPipelineMultisampleStateCreateInfo multisampling = { 0 };
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = NULL; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment = { 0 };
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = { 0 };
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineDepthStencilStateCreateInfo depthStencil = { 0 };
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = (VkStencilOpState){ 0 }; // Optional
    depthStencil.back = (VkStencilOpState){ 0 }; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { 0 };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = (VkDescriptorSetLayout *)&pipeline->vkDescriptorSetLayout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

    VkResult result = vkCreatePipelineLayout(graphics->vkDevice, &pipelineLayoutInfo, NULL, (VkPipelineLayout *)&pipeline->vkPipelineLayout);

    if (result != VK_SUCCESS)
    {
        WriteWarning("Failed to create pipeline layout");
        return false;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = { 0 };
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = stageCount;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    if (useTesselation)
        pipelineInfo.pTessellationState = &tesselation;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipeline->vkPipelineLayout;
    pipelineInfo.renderPass = graphics->vkRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    result = vkCreateGraphicsPipelines(graphics->vkDevice, NULL, 1, &pipelineInfo, NULL, (VkPipeline *)&pipeline->vkGraphicsPipeline);
    if (result != VK_SUCCESS)
    {
        WriteWarning("Failed to create graphics pipeline");
        return false;
    }

    free(shaderStages);

    return true;
}