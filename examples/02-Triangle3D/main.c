#include <ShlibVK/ShlibVK.h>
#include <stdio.h>

typedef struct sVertex
{
    Vec3 position;
} Vertex;

typedef struct sUniformMatrices
{
    Matrix projection;
    Matrix view;
} UniformMatrices;

const Vertex vertices[] =
        {
                {{ 0.0f, 0.0f, -0.5f }},
                {{ 0.5f, 0.0f,  0.5f }},
                {{-0.5f, 0.0f,  0.5f }},
        };

Window gWindow;
Graphics gGraphics;
Pipeline gPipeline;
Mesh gMesh;
UniformBuffer gUniformBuffer;
UniformMatrices gUniforms;

void InitWindow();
void InitGraphics();
void InitPipeline();
void InitMesh();
void Cleanup();

int main()
{
    gUniforms.projection = MatrixPerspective(800.0f / 600.0f, 45, 0.01f, 1000.0f);
    gUniforms.view = MatrixLookAt((Vec3){0, 1, 5}, (Vec3){0, 0, 0}, (Vec3){0, 1, 0});

    InitWindow();
    InitGraphics();
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
    createInfo.pTitle = "02 - Triangle 3D";

    WindowCreate(&createInfo, &gWindow);
}

void InitGraphics()
{
    GraphicsCreateInfo createInfo = { 0 };
    createInfo.debug = true;
    createInfo.pAppName = "02 - Triangle 3D";
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

    Attribute position = { 0 };
    position.location = 0;
    position.offset = 0;
    position.components = 3;

    PipelineCreateInfo createInfo = { 0 };
    createInfo.topology = TOPOLOGY_TRIANGLE_LIST;
    int size = 0;
    createInfo.pVertexShaderCode = FileReadBytes("../../resources/bin/quad.vert", &size);
    createInfo.vertexShaderSize = size;
    createInfo.pFragmentShaderCode = FileReadBytes("../../resources/bin/quad.frag", &size);
    createInfo.fragmentShaderSize = size;
    createInfo.stride = sizeof(Vertex);
    createInfo.attributeCount = 1;
    createInfo.pAttributes = &position;
    createInfo.descriptorCount = 1;
    createInfo.pDescriptors = &matrices;
    createInfo.uniformBufferCount = 1;
    createInfo.pUniformBuffers = &gUniformBuffer;

    PipelineCreate(gGraphics, &createInfo, &gPipeline);
}

void InitMesh()
{
    MeshCreateInfo createInfo = { 0 };
    createInfo.stride = sizeof(Vertex);
    createInfo.vertexCount = 3;
    createInfo.pVertices = (float *)vertices;

    MeshCreate(gGraphics, &createInfo, &gMesh);
}

void Cleanup()
{
    UniformBufferDestroy(gGraphics, gUniformBuffer);
    MeshDestroy(gGraphics, gMesh);
    PipelineDestroy(gGraphics, gPipeline);
    GraphicsDestroy(gGraphics);
    WindowDestroy(gWindow);
}