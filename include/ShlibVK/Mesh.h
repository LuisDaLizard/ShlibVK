#ifndef SHLIBVK_MESH_H
#define SHLIBVK_MESH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Graphics.h"
#include "Buffer.h"

struct sMeshCreateInfo {
    unsigned int stride;
    unsigned int vertexCount;

    float *pVertices;
};

struct sMesh {
    unsigned int vertexCount;

    Buffer vertexBuffer;
};

typedef struct sMeshCreateInfo MeshCreateInfo;
typedef struct sMesh *Mesh;

bool MeshCreate(Graphics graphics, MeshCreateInfo *pCreateInfo, Mesh *pMesh);

void MeshDestroy(Graphics graphics, Mesh mesh);

void MeshDraw(Graphics graphics, Mesh mesh);

#ifdef __cplusplus
}
#endif

#endif //SHLIBVK_MESH_H
