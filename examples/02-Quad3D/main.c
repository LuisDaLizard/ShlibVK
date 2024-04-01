#include <ShlibVK/ShlibVK.h>
#include <stdio.h>

const char *vertSource = "#version 450\n"
                         "\n"
                         "layout(location = 0) in vec3 aPosition;\n"
                         "\n"
                         "layout(binding = 0) uniform UniformBuffer\n"
                         "{\n"
                         "\tmat4 uView;\n"
                         "\tmat4 uProjection;\n"
                         "} Uniforms;\n"
                         "\n"
                         "void main()\n"
                         "{\n"
                         "\tgl_Position = Uniforms.uProjection * Uniforms.uView * vec4(aPosition, 1);\n"
                         "}";

const char *fragSource = "#version 450\n"
                         "\n"
                         "layout(location = 0) out vec4 oColor;\n"
                         "\n"
                         "void main()\n"
                         "{\n"
                         "\toColor = vec4(1, 1, 1, 1);\n"
                         "}";

typedef struct sVertex
{
    Vec3 position;
} Vertex;

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
    createInfo.pVertexShaderCode = vertSource;
    createInfo.pFragmentShaderCode = fragSource;
    createInfo.attributeCount = 1;
    createInfo.pAttributes = &position;
    createInfo.descriptorCount = 1;
    createInfo.pDescriptors = &matrices;
    createInfo.stride = sizeof(Vertex);

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
    MeshDestroy(gGraphics, gMesh);
    PipelineDestroy(gGraphics, gPipeline);
    GraphicsDestroy(gGraphics);
    WindowDestroy(gWindow);
}