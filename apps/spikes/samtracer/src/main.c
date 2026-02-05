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

#include "ppm.h"
#include "ray.h"
#include <samrena.h>
#include <stdbool.h>
#include <stdio.h>

#include <math.h>

double hit_sphere(Point3 center, double radius, Ray r) {
  Vec3 oc = vec3_sub(center, r.origin);
  double a = vec3_squared_length(r.direction);
  double h = vec3_dot(r.direction, oc);
  double c = vec3_squared_length(oc) - radius * radius;

  double discriminant = h * h - a * c;

  if (discriminant < 0) {
    return -1.0;
  } else {
    return (h - sqrt(discriminant)) / (2.0 * a);
  }
}

Vec3 ray_colour(Ray r) {
  Vec3 sphere_center = {0.0, 0.0, -1.0};
  double t = hit_sphere(sphere_center, 0.5, r);
  if (t > 0.0) {
    Vec3 N = vec3_unit(vec3_sub(ray_at(r, t), sphere_center));
    Vec3 return_color = {N.x + 1, N.y + 1, N.z + 1};
    return vec3_mul(return_color, 0.5);
  }

  Vec3 unit_direction = vec3_unit(r.direction);
  double a = 0.5 * (unit_direction.y + 1.0);

  Vec3 color_blue = {0.5, 0.7, 1.0};
  Vec3 color_white = {1.0, 1.0, 1.0};

  return vec3_add(vec3_mul(color_white, 1 - a), vec3_mul(color_blue, a));
}

int main(void) {
  Samrena *arena = samrena_create_default();

  // Image
  double aspect_ratio = 16.0 / 9.0;
  int image_width = 400;
  int image_height = (int)(image_width / aspect_ratio);
  Image *img = image_create(arena, image_width, image_height);

  // Camera

  double focal_length = 1.0;
  double viewport_height = 2.0;
  double viewport_width = viewport_height * ((double)image_width) / image_height;
  Vec3 camera_center = {0.0, 0.0, 0.0};

  // Calculate the vectors across the horizontal and down the vertical viewport edges.
  Vec3 viewport_u = {viewport_width, 0.0, 0.0};
  Vec3 viewport_v = {0.0, -viewport_height, 0.0};

  Vec3 pixel_delta_u = vec3_div(viewport_u, image_width);
  Vec3 pixel_delta_v = vec3_div(viewport_v, image_height);

  Vec3 focal_vec = {0.0, 0.0, focal_length};

  // viewport_upper_left = camera - focal - u/2 - v/2
  Vec3 viewport_upper_left =
      vec3_sub(vec3_sub(vec3_sub(camera_center, focal_vec), vec3_div(viewport_u, 2)),
               vec3_div(viewport_v, 2));

  Vec3 pixel00_loc =
      vec3_add(viewport_upper_left, vec3_mul(vec3_add(pixel_delta_u, pixel_delta_v), 0.5));

  // Render
  for (int j = 0; j < image_height; j++) {
    printf("\rScanlines remaining: %d ", image_height - j);
    for (int i = 0; i < image_width; i++) {
      Vec3 pixel_center =
          vec3_add(pixel00_loc, vec3_add(vec3_mul(pixel_delta_u, i), vec3_mul(pixel_delta_v, j)));
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
