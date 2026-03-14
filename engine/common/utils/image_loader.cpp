#include "image_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned char *loadImage(const char *file, int *texWidth, int *texHeight,
                         int *texChannels) {
  stbi_uc *pixels =
      stbi_load(file, texWidth, texHeight, texChannels, STBI_rgb_alpha);
  return pixels;
}

void freeImage(unsigned char *pixels) {
    stbi_image_free(pixels);
}
