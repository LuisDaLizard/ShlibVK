#include "ShlibVK/Utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

FILE *pInfoOutput = NULL;
FILE *pWarningOutput = NULL;
FILE *pErrorOutput = NULL;

void WriteInfo(const char *message)
{
    if (!pInfoOutput)
        pInfoOutput = stdout;

    fprintf(pInfoOutput, "%s\n", message);
}

void WriteWarning(const char *message)
{
    if (!pWarningOutput)
        pWarningOutput = stdout;

    fprintf(pWarningOutput, "WARNING: %s\n", message);
}

void WriteError(int code, const char *message)
{
    if (!pErrorOutput)
        pErrorOutput = stderr;

    fprintf(pErrorOutput, "ERROR (%04x): %s\n", code, message);
    exit(code);
}

char *FileReadText(const char *filePath)
{
    FILE *file = fopen(filePath, "rb");

    if (!file)
        return NULL;

    size_t size;

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *text = malloc(size + 1);
    fread(text, size, 1, file);
    text[size] = 0;

    fclose(file);

    return text;
}

void *FileReadBytes(const char *filePath, int *size)
{
    FILE *file = fopen(filePath, "rb");

    if (!file)
        return NULL;

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    void *buffer = malloc(*size);
    fread(buffer, *size, 1, file);

    fclose(file);

    return buffer;
}

void FileFree(void *contents)
{
    free(contents);
}

double GetTime()
{
    return glfwGetTime();
}