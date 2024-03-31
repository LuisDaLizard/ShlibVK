#ifndef SHLIBVK_WINDOW_H
#define SHLIBVK_WINDOW_H

#include <stdbool.h>

struct sWindowCreateInfo
{
    int width;
    int height;
    const char *pTitle;
};

struct sWindow
{
    int width;
    int height;
    void *pHandle;
};

typedef struct sWindowCreateInfo WindowCreateInfo;
typedef struct sWindow *Window;

bool WindowCreate(WindowCreateInfo *pCreateInfo, Window *pWindow);
void WindowDestroy(Window window);

bool WindowShouldClose(Window window);
void WindowPollEvents(Window window);

#endif //SHLIBVK_WINDOW_H
