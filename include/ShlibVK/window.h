#pragma once

#include "defines.h"
#include "util/result.h"

typedef struct shWindow shWindow_t;

typedef struct shWindowInfo
{
    i32_t width, height;
    const char *title;

    b8_t resizable;
} shWindowInfo_t;

shResult_t shWindowCreate(shWindow_t *window, shWindowInfo_t info);
void shWindowDestroy(shWindow_t *window);

i32_t shWindowGetWidth(shWindow_t *window);
i32_t shWindowGetHeight(shWindow_t *window);

b32_t shWindowShouldClose(shWindow_t *window);