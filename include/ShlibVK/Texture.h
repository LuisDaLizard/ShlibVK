#ifndef SHLIBVK_TEXTURE_H
#define SHLIBVK_TEXTURE_H

#include "Graphics.h"

struct sTextureCreateInfo
{
    unsigned int width;
    unsigned int height;
    unsigned int channels;
    void *pData;
};

struct sTexture
{
    unsigned int width;
    unsigned int height;

    unsigned int format;

    void *vkImage;
    void *vkImageView;
    void *vkImageMemory;
    void *vkSampler;
};

typedef struct sTextureCreateInfo TextureCreateInfo;
typedef struct sTexture *Texture;

bool TextureCreate(Graphics graphics, TextureCreateInfo *pCreateInfo, Texture *pTexture);
void TextureDestroy(Graphics graphics, Texture texture);

#endif //SHLIBVK_TEXTURE_H
