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

typedef struct sAttribute
{
    unsigned int location;
    unsigned int components;
    unsigned int offset;
} Attribute;

struct sPipelineCreateInfo
{
    const char *pVertexShaderCode;
    const char *pFragmentShaderCode;

    Topology topology;

    unsigned int stride;
    unsigned int attributeCount;
    Attribute *pAttributes;
};

struct sPipeline
{
    void *vkPipelineLayout;
    void *vkGraphicsPipeline;
};

typedef struct sPipelineCreateInfo PipelineCreateInfo;
typedef struct sPipeline *Pipeline;

bool PipelineCreate(Graphics graphics, PipelineCreateInfo *pCreateInfo, Pipeline *pPipeline);
void PipelineDestroy(Graphics graphics, Pipeline pipeline);

void PipelineBind(Graphics graphics, Pipeline pipeline);

#endif //SHLIBVK_PIPELINE_H
