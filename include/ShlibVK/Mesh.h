#ifndef SHLIBVK_MESH_H
#define SHLIBVK_MESH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Graphics.h"
#include "Buffer.h"
#include "StorageBuffer.h"

struct sMeshCreateInfo {
    unsigned int vertexCount;
    unsigned int bufferCount;

    unsigned int *strides;
    void **ppData;
};

struct sMesh {
    unsigned int vertexCount;
    unsigned int bufferCount;

    Buffer *pVertexBuffers;
};

typedef struct sMeshCreateInfo MeshCreateInfo;
typedef struct sMesh *Mesh;

bool MeshCreate(Graphics graphics, MeshCreateInfo *pCreateInfo, Mesh *pMesh);

void MeshDestroy(Graphics graphics, Mesh mesh);

Buffer MeshGetBuffer(Mesh mesh, unsigned int index);

StorageBuffer MeshGetStorageBuffer(Mesh mesh, unsigned int index);

void MeshDraw(Graphics graphics, Mesh mesh);

#ifdef __cplusplus
}
#endif

#endif //SHLIBVK_MESH_H
