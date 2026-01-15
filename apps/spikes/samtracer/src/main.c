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

#include <stdio.h>
#include <samrena.h>
#include "ppm.h"

int main(void) {
    Samrena* arena = samrena_create_default();

    int width = 256;
    int height = 256;
    Image* img = image_create(arena, width, height);

    // Background gradient using write_colour
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Vec3 bg = {(double)x / width, (double)y / height, 0.2};
            image_write_colour(img, x, y, bg);
        }
    }

    // Draw a filled red rectangle
    Vec3 red = {1.0, 0.0, 0.0};
    for (int y = 20; y < 60; y++) {
        for (int x = 20; x < 100; x++) {
            image_write_colour(img, x, y, red);
        }
    }

    // Draw a filled circle (green)
    Vec3 green = {0.0, 1.0, 0.0};
    int cx = 180, cy = 180, radius = 40;
    for (int y = cy - radius; y <= cy + radius; y++) {
        for (int x = cx - radius; x <= cx + radius; x++) {
            int dx = x - cx;
            int dy = y - cy;
            if (dx * dx + dy * dy <= radius * radius) {
                image_write_colour(img, x, y, green);
            }
        }
    }

    // Draw a blue diagonal line
    Vec3 blue = {0.0, 0.0, 1.0};
    for (int i = 0; i < 100; i++) {
        image_write_colour(img, 70 + i, 100 + i, blue);
        image_write_colour(img, 71 + i, 100 + i, blue);
    }

    image_ppm_save(img, "test_rgb.ppm");
    printf("Wrote test_rgb.ppm\n");

    samrena_destroy(arena);
    return 0;
}
