#include "ShlibVK/Utilities/List.h"

#include <stdlib.h>
#include <string.h>

void DoubleBackingArray(List list);

List ListCreate(unsigned int elementSize)
{
    List list = malloc(sizeof(struct sList));
    list->elementSize = elementSize;
    list->elementCount = 0;
    list->maxElements = 10;
    list->pData = calloc(elementSize, list->maxElements);

    return list;
}

void ListDestroy(List list)
{
    free(list->pData);
    free(list);
}

void ListAdd(List list, void *pElement)
{
    ListInsert(list, pElement, list->elementCount);
}

void ListInsert(List list, void *pElement, unsigned int index)
{
    if (list->elementCount == list->maxElements)
        DoubleBackingArray(list);

    // Copy any elements after
    memcpy(list->pData + (list->elementSize * (index + 1)), list->pData + (list->elementSize * index), list->elementSize * (list->elementCount - index));

    // Copy element into array
    memcpy(list->pData + (list->elementSize * index), pElement, list->elementSize);

    list->elementCount++;
}

void *ListGet(List list, unsigned int index)
{
    if (index >= list->elementCount)
        return NULL;
    return list->pData + (list->elementSize * index);
}

void DoubleBackingArray(List list)
{
    void *newData = malloc(list->elementSize * list->maxElements * 2);

    memcpy(newData, list->pData, list->elementSize * list->maxElements);
    free(list->pData);

    list->maxElements *= 2;
    list->pData = newData;
}