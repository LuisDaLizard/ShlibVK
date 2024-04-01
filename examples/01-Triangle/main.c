#include <ShlibVK/ShlibVK.h>
#include <stdlib.h>

const char *pVertexSource = "#version 450\n"
                            "\n"
                            "layout(location = 0) in vec2 aPosition;\n"
                            "layout(location = 1) in vec3 aColor;\n"
                            "\n"
                            "layout(location = 0) out vec3 fColor;\n"
                            "\n"
                            "void main() {\n"
                            "    gl_Position = vec4(aPosition, 0.0, 1.0);\n"
                            "    fColor = aColor;\n"
                            "}";
const char *pFragmentSource = "#version 450\n"
                              "\n"
                              "layout(location = 0) in vec3 fColor;\n"
                              "\n"
                              "layout(location = 0) out vec4 oColor;\n"
                              "\n"
                              "void main() {\n"
                              "    oColor = vec4(fColor, 1.0);\n"
                              "}";

typedef struct sVertex
{
    float x;
    float y;

    float r;
    float g;
    float b;
} Vertex;

const Vertex vertices[] =
        {
                { 0.0f, -0.5f, 1.0f, 1.0f, 1.0f},
                { 0.5f,  0.5f, 0.0f, 1.0f, 0.0f},
                {-0.5f,  0.5f, 0.0f, 0.0f, 1.0f},
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
        GraphicsBindPipeline(gGraphics, gPipeline);

        MeshDraw(gMesh);

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
    color.offset = offsetof(Vertex, r);
    color.components = 3;

    Attribute attributes[] = {position, color};

    PipelineCreateInfo createInfo = { 0 };
    createInfo.graphics = gGraphics;
    createInfo.topology = TOPOLOGY_TRIANGLE_LIST;
    createInfo.pVertexShaderCode = pVertexSource;
    createInfo.pFragmentShaderCode = pFragmentSource;
    createInfo.attributeCount = 2;
    createInfo.pAttributes = attributes;
    createInfo.stride = sizeof(Vertex);

    PipelineCreate(&createInfo, &gPipeline);
}

void InitMesh()
{
    MeshCreateInfo createInfo = { 0 };
    createInfo.stride = sizeof(Vertex);
    createInfo.vertexCount = 3;
    createInfo.pVertices = (float *)vertices;
    createInfo.graphics = gGraphics;

    MeshCreate(&createInfo, &gMesh);
}

void Cleanup()
{
    MeshDestroy(gMesh);
    PipelineDestroy(gPipeline);
    GraphicsDestroy(gGraphics);
    WindowDestroy(gWindow);
}