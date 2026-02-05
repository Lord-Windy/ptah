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

#include "ray.h"
#include <math.h>
#include <stdio.h>

#define EPSILON 1e-9

static int tests_run = 0;
static int tests_passed = 0;

static int double_eq(double a, double b) { return fabs(a - b) < EPSILON; }

static int vec3_eq(Vec3 a, Vec3 b) {
  return double_eq(a.x, b.x) && double_eq(a.y, b.y) && double_eq(a.z, b.z);
}

#define TEST(name) static int name(void)

#define RUN_TEST(name)                                                                             \
  do {                                                                                             \
    tests_run++;                                                                                   \
    if (name()) {                                                                                  \
      tests_passed++;                                                                              \
      printf("  PASS: %s\n", #name);                                                               \
    } else {                                                                                       \
      printf("  FAIL: %s\n", #name);                                                               \
    }                                                                                              \
  } while (0)

TEST(test_ray_at) {
  Point3 origin = {1.0, 2.0, 3.0};
  Vec3 direction = {4.0, 5.0, 6.0};
  Ray r = {origin, direction};

  // r.at(0) = origin
  if (!vec3_eq(ray_at(r, 0.0), origin))
    return 0;

  // r.at(1) = origin + direction = {5, 7, 9}
  Point3 p1 = {5.0, 7.0, 9.0};
  if (!vec3_eq(ray_at(r, 1.0), p1))
    return 0;

  // r.at(-1) = origin - direction = {-3, -3, -3}
  Point3 pm1 = {-3.0, -3.0, -3.0};
  if (!vec3_eq(ray_at(r, -1.0), pm1))
    return 0;

  // r.at(0.5) = origin + 0.5 * direction = {1+2, 2+2.5, 3+3} = {3, 4.5, 6}
  Point3 p05 = {3.0, 4.5, 6.0};
  if (!vec3_eq(ray_at(r, 0.5), p05))
    return 0;

  return 1;
}

int ray_run_tests(void) {
  tests_run = 0;
  tests_passed = 0;

  printf("Running ray tests...\n\n");

  RUN_TEST(test_ray_at);

  printf("\n%d/%d tests passed\n", tests_passed, tests_run);

  return tests_passed == tests_run;
}
