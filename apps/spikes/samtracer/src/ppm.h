//
// Created by sam on 15/1/26.
//

#ifndef PTAH_PPM_H
#define PTAH_PPM_H

#include <stdio.h>
#include <stdint.h>

#include <samrena.h>

typedef struct {
  int width;
  int height;
  uint8_t *pixels;
} Image;

Image* image_create(Samrena* arena, int width, int height);

void image_ppm_write(Image* image, const char* filename);

#endif // PTAH_PPM_H
