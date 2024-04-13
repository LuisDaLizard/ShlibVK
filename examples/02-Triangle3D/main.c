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

    Descriptor uniforms = { 0 };
    uniforms.location = 0;
    uniforms.count = 1;
    uniforms.stage = STAGE_VERTEX;
    uniforms.type = DESCRIPTOR_TYPE_UNIFORM;
    uniforms.uniform = gUniformBuffer;

    Attribute position = { 0 };
    position.location = 0;
    position.binding = 0;
    position.offset = 0;
    position.components = 3;

    InputBinding binding = { 0 };
    binding.binding = 0;
    binding.stride = sizeof(Vertex);

    PipelineCreateInfo createInfo = { 0 };
    createInfo.topology = TOPOLOGY_TRIANGLE_LIST;
    int size = 0;
    createInfo.pVertexShaderCode = FileReadBytes("../../resources/shaders/bin/quad.vert", &size);
    createInfo.vertexShaderSize = size;
    createInfo.pFragmentShaderCode = FileReadBytes("../../resources/shaders/bin/quad.frag", &size);
    createInfo.fragmentShaderSize = size;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &binding;
    createInfo.attributeCount = 1;
    createInfo.pAttributes = &position;
    createInfo.descriptorCount = 1;
    createInfo.pDescriptors = &uniforms;

    PipelineCreate(gGraphics, &createInfo, &gPipeline);
}

void InitMesh()
{
    unsigned int stride = sizeof(Vertex);

    MeshCreateInfo createInfo = { 0 };
    createInfo.vertexCount = 3;
    createInfo.bufferCount = 1;
    createInfo.strides = &stride;
    createInfo.ppData = (void **)&vertices;

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