#include <ShlibVK/ShlibVK.h>
#include <stdlib.h>

const char *pVertexSource = "#version 450\n"
                            "layout (location = 0) out vec3 fColor;\n"
                            "vec2 positions[3] = vec2[](\n"
                            "    vec2(0.0, -0.5),\n"
                            "    vec2(0.5, 0.5),\n"
                            "    vec2(-0.5, 0.5)\n"
                            ");\n"
                            "\n"
                            "vec3 colors[3] = vec3[](\n"
                            "    vec3(1.0, 0.0, 0.0),\n"
                            "    vec3(0.0, 1.0, 0.0),\n"
                            "    vec3(0.0, 0.0, 1.0)\n"
                            ");\n"
                            "\n"
                            "void main() {\n"
                            "    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);\n"
                            "    fColor = colors[gl_VertexIndex];\n"
                            "}";
const char *pFragmentSource = "#version 450\n"
                              "\n"
                              "layout(location = 0) in vec3 fColor;\n"
                              "layout(location = 0) out vec4 oColor;\n"
                              "\n"
                              "void main() {\n"
                              "    oColor = vec4(fColor, 1.0);\n"
                              "}";

Window gWindow;
Graphics gGraphics;
Pipeline gPipeline;

void InitWindow();
void InitGraphics();
void InitPipeline();
void Cleanup();

int main()
{
    InitWindow();
    InitGraphics();
    InitPipeline();

    while(!WindowShouldClose(gWindow))
    {
        WindowPollEvents(gWindow);

        GraphicsBeginRenderPass(gGraphics);
        GraphicsBindPipeline(gGraphics, gPipeline);
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
    PipelineCreateInfo createInfo = { 0 };
    createInfo.graphics = gGraphics;
    createInfo.topology = TOPOLOGY_TRIANGLE_LIST;
    createInfo.pVertexShaderCode = pVertexSource;
    createInfo.pFragmentShaderCode = pFragmentSource;

    PipelineCreate(&createInfo, &gPipeline);
}

void Cleanup()
{
    PipelineDestroy(gPipeline);
    GraphicsDestroy(gGraphics);
    WindowDestroy(gWindow);
}