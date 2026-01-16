/*
 * Copyright 2025 Samuel "Lord-Windy" Brown
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdbool.h>
#include <stdio.h>
#include <samrena.h>
#include "ppm.h"
#include "ray.h"

bool hit_sphere(Point3 center, double radius, Ray r) {
  Vec3 oc = vec3_sub(center, r.origin);
  double a = vec3_dot(r.direction, r.direction);
  double b = -2.0 * vec3_dot(r.direction, oc);
  double c = vec3_dot(oc, oc) - radius * radius;

  double discriminant = b * b - 4.0 * a * c;
  return (discriminant >= 0);
}

Vec3 ray_colour(Ray r) {
  Vec3 sphere_center = {0.0, 0.0, -1.0};
  if (hit_sphere(sphere_center, 0.5, r)) {
    Vec3 red = {1.0, 0, 0};
    return red;
  }

  Vec3 unit_direction = vec3_unit(r.direction);
  double a = 0.5 * (unit_direction.y + 1.0);

  Vec3 color_blue = {0.5, 0.7, 1.0};
  Vec3 color_white = {1.0, 1.0, 1.0};

  return vec3_add(vec3_mul(color_white, 1-a), vec3_mul(color_blue, a));
}

int main(void) {
  Samrena* arena = samrena_create_default();

  // Image
  double aspect_ratio = 16.0 / 9.0;
  int image_width = 400;
  int image_height = (int)(image_width / aspect_ratio);
  Image* img = image_create(arena, image_width, image_height);

  //Camera

  double focal_length = 1.0;
  double viewport_height = 2.0;
  double viewport_width = viewport_height * ((double) image_width)/image_height;
  Vec3 camera_center = {0.0, 0.0, 0.0};

  // Calculate the vectors across the horizontal and down the vertical viewport edges.
  Vec3 viewport_u = { viewport_width, 0.0, 0.0 };
  Vec3 viewport_v = { 0.0, -viewport_height, 0.0 };

  Vec3 pixel_delta_u = vec3_div(viewport_u, image_width);
  Vec3 pixel_delta_v = vec3_div(viewport_v, image_height);

  Vec3 focal_vec = {0.0, 0.0,focal_length};

  // viewport_upper_left = camera - focal - u/2 - v/2
  Vec3 viewport_upper_left = vec3_sub(
    vec3_sub(vec3_sub(camera_center, focal_vec), vec3_div(viewport_u, 2)),
    vec3_div(viewport_v, 2));

  Vec3 pixel00_loc = vec3_add(viewport_upper_left,
    vec3_mul(vec3_add(pixel_delta_u, pixel_delta_v), 0.5));


  // Render
  for (int j = 0; j < image_height; j++) {
    fprintf(stderr, "\rScanlines remaining: %d ", image_height - j);
    for (int i = 0; i < image_width; i++) {
      Vec3 pixel_center = vec3_add(pixel00_loc,
        vec3_add(vec3_mul(pixel_delta_u, i), vec3_mul(pixel_delta_v, j)));
      Vec3 ray_direction = vec3_sub(pixel_center, camera_center);
      Ray r = {camera_center, ray_direction};

      Vec3 pixel_color = ray_colour(r);
      image_write_colour(img, i, j, pixel_color);
    }
  }
  fprintf(stderr, "\rDone.                    \n");


  image_ppm_save(img, "test_rgb.ppm");
  printf("Wrote test_rgb.ppm\n");

  samrena_destroy(arena);
  return 0;
}
