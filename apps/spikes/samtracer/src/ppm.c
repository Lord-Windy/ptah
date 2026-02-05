//
// Created by sam on 15/1/26.
//

#include "ppm.h"

#include <math.h>

Image *image_create(Samrena *arena, int width, int height) {

  Image *img = SAMRENA_PUSH_TYPE(arena, Image);

  img->width = width;
  img->height = height;

  img->pixels = SAMRENA_PUSH_ARRAY(arena, uint8_t, width * height * 3);

  return img;
}

void image_ppm_save_header(Image *image, FILE *fp) {
  fprintf(fp, "P3\n%d %d\n255\n", image->width, image->height);
}

void image_ppm_save(Image *image, const char *filename) {
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    return;
  }

  image_ppm_save_header(image, fp);

  for (int y = 0; y < image->height; y++) {
    for (int x = 0; x < image->width; x++) {
      int idx = (y * image->width + x) * 3;
      fprintf(fp, "%d %d %d\n", image->pixels[idx], image->pixels[idx + 1], image->pixels[idx + 2]);
    }
  }

  fclose(fp);
}

void image_write_colour(Image *image, int x, int y, Vec3 colour) {
  int idx = (y * image->width + x) * 3;
  image->pixels[idx] = (uint8_t)floor(colour.x * 255.0);
  image->pixels[idx + 1] = (uint8_t)floor(colour.y * 255.0);
  image->pixels[idx + 2] = (uint8_t)floor(colour.z * 255.0);
}