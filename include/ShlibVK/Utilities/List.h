#ifndef SHLIBVK_LIST_H
#define SHLIBVK_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

struct sList {
    unsigned int elementSize;
    unsigned int elementCount;

    char *pData;
    unsigned int maxElements;
};

typedef struct sList *List;

List ListCreate(unsigned int elementSize);

void ListDestroy(List list);

void ListAdd(List list, void *pElement);

void ListInsert(List list, void *pElement, unsigned int index);

void *ListGet(List list, unsigned int index);

#ifdef __cplusplus
}
#endif

#endif //SHLIBVK_LIST_H
