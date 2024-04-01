#ifndef SHLIBVK_MESH_H
#define SHLIBVK_MESH_H

#include "Graphics.h"

struct sMeshCreateInfo
{
    Graphics graphics;

    unsigned int stride;
    unsigned int vertexCount;

    float *pVertices;
};

struct sMesh
{
    Graphics graphics;

    unsigned int vertexCount;

    void *vkBuffer;
    void *vkDeviceMemory;
};

typedef struct sMeshCreateInfo MeshCreateInfo;
typedef struct sMesh *Mesh;

bool MeshCreate(MeshCreateInfo *pCreateInfo, Mesh *pMesh);
void MeshDestroy(Mesh mesh);

void MeshDraw(Mesh mesh);

#endif //SHLIBVK_MESH_H
