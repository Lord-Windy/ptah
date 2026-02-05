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

#include "vec3.h"

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

TEST(test_vec3_add) {
  Vec3 a = {1.0, 2.0, 3.0};
  Vec3 b = {4.0, 5.0, 6.0};
  Vec3 result = vec3_add(a, b);
  Vec3 expected = {5.0, 7.0, 9.0};
  return vec3_eq(result, expected);
}

TEST(test_vec3_sub) {
  Vec3 a = {5.0, 7.0, 9.0};
  Vec3 b = {1.0, 2.0, 3.0};
  Vec3 result = vec3_sub(a, b);
  Vec3 expected = {4.0, 5.0, 6.0};
  return vec3_eq(result, expected);
}

TEST(test_vec3_mul) {
  Vec3 a = {1.0, 2.0, 3.0};
  Vec3 result = vec3_mul(a, 2.0);
  Vec3 expected = {2.0, 4.0, 6.0};
  return vec3_eq(result, expected);
}

TEST(test_vec3_div) {
  Vec3 a = {2.0, 4.0, 6.0};
  Vec3 result = vec3_div(a, 2.0);
  Vec3 expected = {1.0, 2.0, 3.0};
  return vec3_eq(result, expected);
}

TEST(test_vec3_dot) {
  Vec3 a = {1.0, 2.0, 3.0};
  Vec3 b = {4.0, 5.0, 6.0};
  double result = vec3_dot(a, b);
  // dot product = 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
  return double_eq(result, 32.0);
}

TEST(test_vec3_cross) {
  Vec3 a = {1.0, 0.0, 0.0};
  Vec3 b = {0.0, 1.0, 0.0};
  Vec3 result = vec3_cross(a, b);
  Vec3 expected = {0.0, 0.0, 1.0};
  return vec3_eq(result, expected);
}

TEST(test_vec3_cross_anticommutative) {
  Vec3 a = {1.0, 2.0, 3.0};
  Vec3 b = {4.0, 5.0, 6.0};
  Vec3 ab = vec3_cross(a, b);
  Vec3 ba = vec3_cross(b, a);
  // cross product is anticommutative: a x b = -(b x a)
  Vec3 neg_ba = vec3_mul(ba, -1.0);
  return vec3_eq(ab, neg_ba);
}

TEST(test_vec3_length) {
  Vec3 a = {3.0, 4.0, 0.0};
  double result = vec3_length(a);
  return double_eq(result, 5.0);
}

TEST(test_vec3_squared_length) {
  Vec3 a = {3.0, 4.0, 0.0};
  double result = vec3_squared_length(a);
  return double_eq(result, 25.0);
}

TEST(test_vec3_unit) {
  Vec3 a = {3.0, 0.0, 0.0};
  Vec3 result = vec3_unit(a);
  Vec3 expected = {1.0, 0.0, 0.0};
  return vec3_eq(result, expected);
}

TEST(test_vec3_unit_length_is_one) {
  Vec3 a = {1.0, 2.0, 3.0};
  Vec3 unit = vec3_unit(a);
  double len = vec3_length(unit);
  return double_eq(len, 1.0);
}

TEST(test_vec3_distance) {
  Vec3 a = {0.0, 0.0, 0.0};
  Vec3 b = {3.0, 4.0, 0.0};
  double result = vec3_distance(a, b);
  return double_eq(result, 5.0);
}

TEST(test_vec3_distance_symmetric) {
  Vec3 a = {1.0, 2.0, 3.0};
  Vec3 b = {4.0, 5.0, 6.0};
  double ab = vec3_distance(a, b);
  double ba = vec3_distance(b, a);
  return double_eq(ab, ba);
}

int vec3_run_tests(void) {
  tests_run = 0;
  tests_passed = 0;

  printf("Running vec3 tests...\n\n");

  RUN_TEST(test_vec3_add);
  RUN_TEST(test_vec3_sub);
  RUN_TEST(test_vec3_mul);
  RUN_TEST(test_vec3_div);
  RUN_TEST(test_vec3_dot);
  RUN_TEST(test_vec3_cross);
  RUN_TEST(test_vec3_cross_anticommutative);
  RUN_TEST(test_vec3_length);
  RUN_TEST(test_vec3_squared_length);
  RUN_TEST(test_vec3_unit);
  RUN_TEST(test_vec3_unit_length_is_one);
  RUN_TEST(test_vec3_distance);
  RUN_TEST(test_vec3_distance_symmetric);

  printf("\n%d/%d tests passed\n", tests_passed, tests_run);

  return tests_passed == tests_run;
}
