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
    STAGE_FRAGMENT = 0x10,
} ShaderStage;

typedef enum eDescriptorType {
    DESCRIPTOR_TYPE_UNIFORM = 0,
    DESCRIPTOR_TYPE_SAMPLER,
} DescriptorType;

typedef struct sAttribute {
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
    const unsigned int *pFragmentShaderCode;
    unsigned int fragmentShaderSize;

    Topology topology;

    unsigned int stride;
    unsigned int attributeCount;
    Attribute *pAttributes;

    unsigned int descriptorCount;
    Descriptor *pDescriptors;
};

struct sPipeline {
    void *vkPipelineLayout;
    void *vkDescriptorPool;
    void *vkGraphicsPipeline;
    void *vkDescriptorSetLayout;
    void *vkDescriptorSet;
};

typedef struct sPipelineCreateInfo PipelineCreateInfo;
typedef struct sPipeline *Pipeline;

bool PipelineCreate(Graphics graphics, PipelineCreateInfo *pCreateInfo, Pipeline *pPipeline);

void PipelineDestroy(Graphics graphics, Pipeline pipeline);

void PipelineBind(Graphics graphics, Pipeline pipeline);

#ifdef __cplusplus
}
#endif

#endif //SHLIBVK_PIPELINE_H
