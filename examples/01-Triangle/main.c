#include <ShlibVK/ShlibVK.h>
#include <stdlib.h>

Window gWindow;
Graphics gGraphics;

void InitWindow();
void InitGraphics();
void Cleanup();

int main()
{
    InitWindow();
    InitGraphics();

    while(!WindowShouldClose(gWindow))
    {
        WindowPollEvents(gWindow);
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

void Cleanup()
{
    WindowDestroy(gWindow);
    GraphicsDestroy(gGraphics);
}