#include <ShlibVK/ShlibVK.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct sVertex
{
    Vec3 position;
    Vec2 uv;
} Vertex;

typedef struct sUniformMatrices
{
    Matrix projection;
    Matrix view;
} UniformMatrices;

const Vertex gVertices[] =
        {
                {{ 0.5f, 0.0f, -0.5f }, {1, 1}},
                {{ 0.5f, 0.0f,  0.5f }, {1, 0}},
                {{-0.5f, 0.0f,  0.5f }, {0, 0}},


                {{-0.5f, 0.0f,  0.5f }, {0, 0}},
                {{-0.5f, 0.0f, -0.5f }, {0, 1}},
                {{ 0.5f, 0.0f, -0.5f }, {1, 1}},
        };

Window gWindow;
Graphics gGraphics;
Pipeline gPipeline;
Mesh gMesh;
UniformBuffer gUniformBuffer;
UniformMatrices gUniforms;
Texture gTexture;

void InitWindow();
void InitGraphics();
void InitPipeline();
void InitMesh();
void InitTexture();
void Cleanup();

int main()
{
    gUniforms.projection = MatrixTranspose(MatrixPerspective(800.0f / 600.0f, 45, 0.01f, 1000.0f));
    gUniforms.view = MatrixTranspose(MatrixLookAt((Vec3){0, 1, -1}, (Vec3){0, 0, 0}, (Vec3){0, -1, 0}));

    InitWindow();
    InitGraphics();
    InitTexture();
    InitPipeline();
    InitMesh();

    while(!WindowShouldClose(gWindow))
    {
        WindowPollEvents(gWindow);

        GraphicsBeginRenderPass(gGraphics);
        PipelineBind(gGraphics, gPipeline);

        MeshDraw(gGraphics, gMesh);

        GraphicsEndRenderPass(gGraphics);
    }

    Cleanup();
    return 0;
}

void InitWindow()
{
    WindowCreateInfo createInfo = { 0 };
    createInfo.width = 800;
    createInfo.height = 600;
    createInfo.pTitle = "03 - Texture";

    WindowCreate(&createInfo, &gWindow);
}

void InitGraphics()
{
    GraphicsCreateInfo createInfo = { 0 };
    createInfo.debug = true;
    createInfo.pAppName = "03 - Texture";
    createInfo.pEngineName = "No Engine";
    createInfo.window = gWindow;

    GraphicsCreate(&createInfo, &gGraphics);
}

void InitPipeline()
{
    UniformBufferCreateInfo bufferInfo = { 0 };
    bufferInfo.binding = 0;
    bufferInfo.size = sizeof(UniformMatrices);

    UniformBufferCreate(gGraphics, &bufferInfo, &gUniformBuffer);
    UniformBufferSetData(gUniformBuffer, &gUniforms, sizeof(UniformMatrices));

    Descriptor matrices = { 0 };
    matrices.location = 0;
    matrices.count = 1;
    matrices.stage = STAGE_VERTEX;
    matrices.type = DESCRIPTOR_TYPE_UNIFORM;
    matrices.uniform = gUniformBuffer;

    Descriptor texture = { 0 };
    texture.location = 1;
    texture.count = 1;
    texture.stage = STAGE_FRAGMENT;
    texture.type = DESCRIPTOR_TYPE_SAMPLER;
    texture.texture = gTexture;

    Attribute position = { 0 };
    position.location = 0;
    position.offset = 0;
    position.components = 3;

    Attribute uv = { 0 };
    uv.location = 1;
    uv.offset = sizeof(Vec3);
    uv.components = 2;

    InputBinding binding = { 0 };
    binding.binding = 0;
    binding.stride = sizeof(Vertex);

    Attribute attributes[2] = {position, uv};
    Descriptor descriptors[2] = {matrices, texture};

    PipelineCreateInfo createInfo = { 0 };
    createInfo.topology = TOPOLOGY_TRIANGLE_LIST;
    int size = 0;
    createInfo.pVertexShaderCode = FileReadBytes("../../resources/shaders/bin/quad.vert", &size);
    createInfo.vertexShaderSize = size;
    createInfo.pFragmentShaderCode = FileReadBytes("../../resources/shaders/bin/quad.frag", &size);
    createInfo.fragmentShaderSize = size;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &binding;
    createInfo.attributeCount = 2;
    createInfo.pAttributes = attributes;
    createInfo.descriptorCount = 2;
    createInfo.pDescriptors = descriptors;

    PipelineCreate(gGraphics, &createInfo, &gPipeline);
}

void InitMesh()
{
    unsigned int stride = sizeof(Vertex);
    Vertex *vertices = malloc(sizeof(Vertex) * 6);
    memcpy(vertices, gVertices, sizeof(Vertex) * 6);

    MeshCreateInfo createInfo = { 0 };
    createInfo.vertexCount = 6;
    createInfo.bufferCount = 1;
    createInfo.strides = &stride;
    createInfo.ppData = (void **)&vertices;

    MeshCreate(gGraphics, &createInfo, &gMesh);
}

void InitTexture()
{
    int width, height, channels;
    void *data = stbi_load("../../resources/textures/statue.jpg", &width, &height, &channels, STBI_rgb_alpha);

    if (!data)
        WriteError(1, "Failed to load image");

    TextureCreateInfo textureInfo = { 0 };
    textureInfo.width = width;
    textureInfo.height = height;
    textureInfo.channels = STBI_rgb_alpha;
    textureInfo.pData = data;

    TextureCreate(gGraphics, &textureInfo, &gTexture);
}

void Cleanup()
{
    TextureDestroy(gGraphics, gTexture);
    UniformBufferDestroy(gGraphics, gUniformBuffer);
    MeshDestroy(gGraphics, gMesh);
    PipelineDestroy(gGraphics, gPipeline);
    GraphicsDestroy(gGraphics);
    WindowDestroy(gWindow);
}