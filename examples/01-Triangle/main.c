#include <ShlibVK/ShlibVK.h>
#include <stdlib.h>
#include <string.h>

typedef struct sVertex
{
    float x;
    float y;

    float r;
    float g;
    float b;
} Vertex;

const Vertex gVertices[] =
        {
                { 0.0f, -0.5f, 1.0f, 1.0f, 1.0f},
                {-0.5f,  0.5f, 0.0f, 0.0f, 1.0f},
                { 0.5f,  0.5f, 0.0f, 1.0f, 0.0f},
        };

Window gWindow;
Graphics gGraphics;
Pipeline gPipeline;
Mesh gMesh;

void InitWindow();
void InitGraphics();
void InitPipeline();
void InitMesh();
void Cleanup();

int main()
{
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
    createInfo.pTitle = "01 - Triangle";

    WindowCreate(&createInfo, &gWindow);
}

void InitGraphics()
{
    GraphicsCreateInfo createInfo = { 0 };
    createInfo.debug = true;
    createInfo.pAppName = "01 - Triangle";
    createInfo.pEngineName = "No Engine";
    createInfo.window = gWindow;

    GraphicsCreate(&createInfo, &gGraphics);
}

void InitPipeline()
{
    Attribute position = { 0 };
    position.location = 0;
    position.offset = 0;
    position.components = 2;

    Attribute color = { 0 };
    color.location = 1;
    color.offset = 2 * sizeof(float);
    color.components = 3;

    Attribute attributes[] = {position, color};

    InputBinding binding = { 0 };
    binding.stride = sizeof(Vertex);
    binding.binding = 0;

    PipelineCreateInfo createInfo = { 0 };
    createInfo.topology = TOPOLOGY_TRIANGLE_LIST;
    int size = 0;
    createInfo.pVertexShaderCode = FileReadBytes("../../resources/shaders/bin/triangle.vert", &size);
    createInfo.vertexShaderSize = size;
    createInfo.pFragmentShaderCode = FileReadBytes("../../resources/shaders/bin/triangle.frag", &size);
    createInfo.fragmentShaderSize = size;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &binding;
    createInfo.attributeCount = 2;
    createInfo.pAttributes = attributes;
    createInfo.descriptorCount = 0;
    createInfo.pDescriptors = NULL;

    PipelineCreate(gGraphics, &createInfo, &gPipeline);
}

void InitMesh()
{
    unsigned int stride = sizeof(Vertex);
    Vertex *vertices = malloc(sizeof(Vertex) * 3);
    memcpy(vertices, gVertices, sizeof(Vertex) * 3);

    MeshCreateInfo createInfo = { 0 };
    createInfo.bufferCount = 1;
    createInfo.strides = &stride;
    createInfo.vertexCount = 3;
    createInfo.ppData = &vertices;

    MeshCreate(gGraphics, &createInfo, &gMesh);
}

void Cleanup()
{
    MeshDestroy(gGraphics, gMesh);
    PipelineDestroy(gGraphics, gPipeline);
    GraphicsDestroy(gGraphics);
    WindowDestroy(gWindow);
}