#pragma once

#include "ShlibVK/defines.h"

#define SH_RESULT_MAX_MESSAGE 512

typedef struct shResult
{
    i8_t success;
    char message[SH_RESULT_MAX_MESSAGE];
} shResult_t;

shResult_t shSuccess();
shResult_t shFail(const char *msg);