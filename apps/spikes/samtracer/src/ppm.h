//
// Created by sam on 15/1/26.
//

#ifndef PTAH_PPM_H
#define PTAH_PPM_H

#include <stdint.h>
#include <stdio.h>

#include <samrena.h>

#include "vec3.h"

typedef struct {
  int width;
  int height;
  uint8_t *pixels;
} Image;

Image *image_create(Samrena *arena, int width, int height);

void image_ppm_save(Image *image, const char *filename);

void image_write_colour(Image *image, int x, int y, Vec3 colour);

#endif // PTAH_PPM_H
