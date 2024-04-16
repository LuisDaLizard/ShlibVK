#ifndef SHLIBVK_PIPELINE_H
#define SHLIBVK_PIPELINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Graphics.h"
#include "UniformBuffer.h"
#include <stdbool.h>

typedef enum eTopology {
    TOPOLOGY_POINT_LIST = 1,
    TOPOLOGY_TRIANGLE_LIST = 3,
    TOPOLOGY_PATCH_LIST = 10,
} Topology;

typedef enum eShaderStage {
    STAGE_VERTEX = 0x01,
    STAGE_TESSELATION_CONTROL = 0x02,
    STAGE_TESSELATION_EVALUATION = 0x04,
    STAGE_FRAGMENT = 0x10,
    STAGE_COMPUTE = 0x20,
} ShaderStage;

typedef enum eDescriptorType {
    DESCRIPTOR_TYPE_UNIFORM = 0,
    DESCRIPTOR_TYPE_SAMPLER,
    DESCRIPTOR_TYPE_STORAGE,
} DescriptorType;

typedef struct sInputBinding {
    unsigned int binding;
    unsigned int stride;
} InputBinding;

typedef struct sAttribute {
    unsigned int binding;
    unsigned int location;
    unsigned int components;
    unsigned int offset;
} Attribute;

typedef struct sDescriptor {
    unsigned int location;
    unsigned int count;
    ShaderStage stage;
    DescriptorType type;

    union {
        UniformBuffer uniform;
        Texture texture;
    };

} Descriptor;

struct sPipelineCreateInfo {
    const unsigned int *pVertexShaderCode;
    unsigned int vertexShaderSize;
    const unsigned int *pTessCtrlCode;
    unsigned int tessCtrlShaderSize;
    const unsigned int *pTessEvalCode;
    unsigned int tessEvalShaderSize;
    const unsigned int *pFragmentShaderCode;
    unsigned int fragmentShaderSize;

    Topology topology;
    unsigned int patchSize;

    unsigned int bindingCount;
    InputBinding *pBindings;

    unsigned int attributeCount;
    Attribute *pAttributes;

    unsigned int descriptorCount;
    Descriptor *pDescriptors;
};

struct sPipelineComputeCreateInfo
{
    const unsigned int *pComputeShaderCode;
    unsigned int computeShaderSize;

    unsigned int descriptorCount;
    Descriptor *pDescriptors;
};

struct sPipeline {
    void *vkPipelineLayout;
    void *vkPipeline;
    void *vkDescriptorSetLayout;
    void *vkDescriptorSet;
    void *vkDescriptorPool;

    bool compute;
};

typedef struct sPipelineCreateInfo PipelineCreateInfo;
typedef struct sPipelineComputeCreateInfo PipelineComputeCreateInfo;
typedef struct sPipeline *Pipeline;

bool PipelineCreate(Graphics graphics, PipelineCreateInfo *pCreateInfo, Pipeline *pPipeline);

bool PipelineComputeCreate(Graphics graphics, PipelineComputeCreateInfo *pCreateInfo, Pipeline *pPipeline);

void PipelineDestroy(Graphics graphics, Pipeline pipeline);

void PipelineBind(Graphics graphics, Pipeline pipeline);

void PipelineComputeDispatch(Graphics graphics, Pipeline pipeline, unsigned int x, unsigned int y, unsigned int z);

#ifdef __cplusplus
}
#endif

#endif //SHLIBVK_PIPELINE_H
