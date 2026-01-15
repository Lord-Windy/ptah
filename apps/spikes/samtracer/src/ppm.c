//
// Created by sam on 15/1/26.
//

#include "ppm.h"

Image* image_create(Samrena* arena, int width, int height) {

  Image* img = SAMRENA_PUSH_TYPE(arena, Image);

  img->width = width;
  img->height = height;

  img->pixels = SAMRENA_PUSH_ARRAY(arena, uint8_t, width * height * 3);
}

void image_ppm_write_header(Image* image, FILE* fp) {
  fprintf(fp, "P3\n%d %d\n255\n", image->width, image->height);
}

void image_ppm_write(Image* image, const char* filename) {

  // Open file

  // write header

  // for each height and width, write the pixel at that position

}