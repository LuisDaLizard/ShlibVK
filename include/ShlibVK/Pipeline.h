#ifndef SHLIBVK_PIPELINE_H
#define SHLIBVK_PIPELINE_H

#include "Graphics.h"

typedef enum eTopology
{
    TOPOLOGY_POINT_LIST = 1,
    TOPOLOGY_TRIANGLE_LIST = 3,
    TOPOLOGY_PATCH_LIST = 10,
} Topology;

struct sPipelineCreateInfo
{
    Graphics graphics;

    const char *pVertexShaderCode;
    const char *pFragmentShaderCode;

    Topology topology;
};

struct sPipeline
{
    void *vkDevice;
    void *vkPipelineLayout;
    void *vkGraphicsPipeline;
};

typedef struct sPipelineCreateInfo PipelineCreateInfo;
typedef struct sPipeline *Pipeline;

bool PipelineCreate(PipelineCreateInfo *pCreateInfo, Pipeline *pPipeline);
void PipelineDestroy(Pipeline pipeline);

#endif //SHLIBVK_PIPELINE_H
