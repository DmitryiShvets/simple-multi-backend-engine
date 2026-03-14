#pragma once

unsigned char *loadImage(const char *file, int *texWidth, int *texHeight,
                         int *texChannels);
void freeImage(unsigned char* buffer);
