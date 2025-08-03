#include "ShlibVK/util/result.h"
#include <string.h>

shResult_t shSuccess()
{
    return (shResult_t){1};
}

shResult_t shFail(const char *msg)
{
    shResult_t result;
    result.success = 0;

    // Prevent overflow
    i32_t len = strlen(msg);
    if (len > SH_RESULT_MAX_MESSAGE) len = SH_RESULT_MAX_MESSAGE;

    memcpy(result.message, msg, len);

    return result;
}