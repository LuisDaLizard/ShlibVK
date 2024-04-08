#ifndef SHLIBVK_TEXTURE_H
#define SHLIBVK_TEXTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Graphics.h"

struct sTextureCreateInfo {
    unsigned int width;
    unsigned int height;
    unsigned int channels;
    void *pData;
};

struct sTextureDepthCreateInfo {
    unsigned int width;
    unsigned int height;
};

struct sTexture {
    unsigned int width;
    unsigned int height;

    unsigned int format;

    void *vkImage;
    void *vkImageView;
    void *vkImageMemory;
    void *vkSampler;
};

typedef struct sTextureCreateInfo TextureCreateInfo;
typedef struct sTextureDepthCreateInfo TextureDepthCreateInfo;
typedef struct sTexture *Texture;

bool TextureCreate(Graphics graphics, TextureCreateInfo *pCreateInfo, Texture *pTexture);

bool TextureCreateDepth(Graphics graphics, TextureDepthCreateInfo *pCreateInfo, Texture *pTexture);

void TextureDestroy(Graphics graphics, Texture texture);

#ifdef __cplusplus
}
#endif

#endif //SHLIBVK_TEXTURE_H
