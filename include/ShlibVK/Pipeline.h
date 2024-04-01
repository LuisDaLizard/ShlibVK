#ifndef SHLIBVK_PIPELINE_H
#define SHLIBVK_PIPELINE_H

#include "Graphics.h"
#include <stdbool.h>

typedef enum eTopology
{
    TOPOLOGY_POINT_LIST = 1,
    TOPOLOGY_TRIANGLE_LIST = 3,
    TOPOLOGY_PATCH_LIST = 10,
} Topology;

typedef enum eShaderStage
{
    STAGE_VERTEX = 0x01,
    STAGE_FRAGMENT = 0x10,
} ShaderStage;

typedef struct sAttribute
{
    unsigned int location;
    unsigned int components;
    unsigned int offset;
} Attribute;

typedef struct sDescriptor
{
    unsigned int location;
    unsigned int count;
    ShaderStage stage;
} Descriptor;

struct sPipelineCreateInfo
{
    const unsigned int *pVertexShaderCode;
    unsigned int vertexShaderSize;
    const unsigned int *pFragmentShaderCode;
    unsigned int fragmentShaderSize;

    Topology topology;

    unsigned int stride;
    unsigned int attributeCount;
    Attribute *pAttributes;
    unsigned int descriptorCount;
    Descriptor *pDescriptors;
};

struct sPipeline
{
    void *vkPipelineLayout;
    void *vkGraphicsPipeline;
    void *vkDescriptorSetLayout;
};

typedef struct sPipelineCreateInfo PipelineCreateInfo;
typedef struct sPipeline *Pipeline;

bool PipelineCreate(Graphics graphics, PipelineCreateInfo *pCreateInfo, Pipeline *pPipeline);
void PipelineDestroy(Graphics graphics, Pipeline pipeline);

void PipelineBind(Graphics graphics, Pipeline pipeline);

#endif //SHLIBVK_PIPELINE_H
