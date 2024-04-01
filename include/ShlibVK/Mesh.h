#ifndef SHLIBVK_MESH_H
#define SHLIBVK_MESH_H

#include "Graphics.h"
#include "Buffer.h"

struct sMeshCreateInfo
{
    unsigned int stride;
    unsigned int vertexCount;

    float *pVertices;
};

struct sMesh
{
    unsigned int vertexCount;

    Buffer vertexBuffer;
};

typedef struct sMeshCreateInfo MeshCreateInfo;
typedef struct sMesh *Mesh;

bool MeshCreate(Graphics graphics, MeshCreateInfo *pCreateInfo, Mesh *pMesh);
void MeshDestroy(Graphics graphics, Mesh mesh);

void MeshDraw(Graphics graphics, Mesh mesh);

#endif //SHLIBVK_MESH_H
