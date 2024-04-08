#ifndef SHLIBVK_WINDOW_H
#define SHLIBVK_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef void (*ResizeCallback)(void *pUserData, int width, int height);

struct sWindowCreateInfo {
    int width;
    int height;
    const char *pTitle;

    void *pUserData;

    ResizeCallback resizeCallback;
};

struct sWindow {
    int width;
    int height;
    void *pHandle;

    void *pUserData;

    ResizeCallback resizeCallback;
};

typedef struct sWindowCreateInfo WindowCreateInfo;
typedef struct sWindow *Window;

bool WindowCreate(WindowCreateInfo *pCreateInfo, Window *pWindow);

void WindowDestroy(Window window);

bool WindowShouldClose(Window window);

void WindowPollEvents(Window window);

#ifdef __cplusplus
}
#endif

#endif //SHLIBVK_WINDOW_H
