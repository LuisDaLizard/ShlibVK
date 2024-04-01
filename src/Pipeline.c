#include "ShlibVK/Pipeline.h"
#include "ShlibVK/Utils.h"
#include "ShlibVK/Graphics.h"
#include <vulkan/vulkan.h>
#include <shaderc/shaderc.h>
#include <string.h>
#include <stdlib.h>

const unsigned int *CompileShaderSource(Graphics graphics, PipelineCreateInfo *pCreateInfo, const char *pSource, int *pCodeSize, unsigned int shaderType);
VkShaderModule CreateShaderModule(Graphics graphics, const unsigned int *pShaderCode, int shaderCodeSize);

bool PipelineCreate(Graphics graphics, PipelineCreateInfo *pCreateInfo, Pipeline *pPipeline)
{
    int vertexCodeSize = 0;
    int fragmentCodeSize = 0;
    const unsigned int *vertexCode = CompileShaderSource(graphics, pCreateInfo, pCreateInfo->pVertexShaderCode, &vertexCodeSize, shaderc_glsl_vertex_shader);
    const unsigned int *fragmentCode = CompileShaderSource(graphics, pCreateInfo, pCreateInfo->pFragmentShaderCode, &fragmentCodeSize, shaderc_glsl_fragment_shader);

    if (!vertexCode || !fragmentCode)
        return false;

    VkShaderModule vertex = CreateShaderModule(graphics, vertexCode, vertexCodeSize);
    VkShaderModule fragment = CreateShaderModule(graphics, fragmentCode, fragmentCodeSize);

    VkPipelineShaderStageCreateInfo vertexStage = { 0 };
    vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStage.module = vertex;
    vertexStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentStage = { 0 };
    fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStage.module = fragment;
    fragmentStage.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexStage, fragmentStage};

    VkVertexInputBindingDescription bindingDescription = { 0 };
    bindingDescription.binding = 0;
    bindingDescription.stride = pCreateInfo->stride;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription *attributeDescriptions = malloc(sizeof(VkVertexInputAttributeDescription) * pCreateInfo->attributeCount);
    for (int i = 0; i < pCreateInfo->attributeCount; i++)
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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { 0 };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = NULL; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

    Pipeline pipeline = malloc(sizeof(struct sPipeline));

    VkResult result = vkCreatePipelineLayout(graphics->vkDevice, &pipelineLayoutInfo, NULL, (VkPipelineLayout *)&pipeline->vkPipelineLayout);

    if (result != VK_SUCCESS)
    {
        WriteWarning("Failed to create pipeline layout");
        return false;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = { 0 };
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = NULL; // Optional
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

    vkDestroyShaderModule(graphics->vkDevice, vertex, NULL);
    vkDestroyShaderModule(graphics->vkDevice, fragment, NULL);

    *pPipeline = pipeline;
    return true;
}

void PipelineDestroy(Graphics graphics, Pipeline pipeline)
{
    vkDeviceWaitIdle(graphics->vkDevice);

    vkDestroyPipeline(graphics->vkDevice, pipeline->vkGraphicsPipeline, NULL);
    vkDestroyPipelineLayout(graphics->vkDevice, pipeline->vkPipelineLayout, NULL);

    free(pipeline);
}

void PipelineBind(Graphics graphics, Pipeline pipeline)
{
    vkCmdBindPipeline(graphics->vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vkGraphicsPipeline);
}

const unsigned int *CompileShaderSource(Graphics graphics, PipelineCreateInfo *pCreateInfo, const char *pSource, int *pCodeSize, unsigned int shaderType)
{
    int sourceSize = (int)strlen(pSource);

    shaderc_compilation_result_t result = shaderc_compile_into_spv(graphics->vkShaderCompiler, pSource, sourceSize, shaderType, "Shader", "main", NULL);

    // Error checking result
    shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
    if (status != shaderc_compilation_status_success)
    {
        WriteWarning(shaderc_result_get_error_message(result));
        return NULL;
    }

    *pCodeSize = (int)shaderc_result_get_length(result);
    return (const unsigned int *)shaderc_result_get_bytes(result);
}

VkShaderModule CreateShaderModule(Graphics graphics, const unsigned int *pShaderCode, int shaderCodeSize)
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