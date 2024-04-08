#ifndef SHLIBVK_WINDOW_H
#define SHLIBVK_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum eKeyCode
{
    KEY_SPACE = 32,
    KEY_A = 65,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_RIGHT = 262,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
} KeyCode;

typedef enum eMouseButton
{
    MOUSE_LEFT = 0,
    MOUSE_RIGHT,
    MOUSE_MIDDLE,
} MouseButton;

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

bool WindowIsKeyDown(Window window, KeyCode key);

bool WindowIsMouseButtonDown(Window window, MouseButton button);

void WindowGetMousePos(Window window, float *x, float *y);

#ifdef __cplusplus
}
#endif

#endif //SHLIBVK_WINDOW_H
