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

#include <assert.h>
#include <math.h>
#include <samdata.h>
#include <stdio.h>
#include <stdlib.h>

static void test_samrng_creation(void) {
  Samrena *arena = samrena_create_default();
  assert(arena != NULL);

  SamRng *rng = samrng_create(arena, 12345);
  assert(rng != NULL);

  samrng_destroy(rng);
  samrena_destroy(arena);
  printf("PASS: RNG creation test\n");
}

static void test_samrng_basic_generation(void) {
  Samrena *arena = samrena_create_default();
  SamRng *rng = samrng_create(arena, 42);

  uint32_t val1 = samrng_uint32(rng);
  uint32_t val2 = samrng_uint32(rng);
  assert(val1 != val2);

  uint64_t val64 = samrng_uint64(rng);
  assert(val64 != 0);

  float fval = samrng_float(rng);
  assert(fval >= 0.0f && fval <= 1.0f);

  double dval = samrng_double(rng);
  assert(dval >= 0.0 && dval <= 1.0);

  samrng_destroy(rng);
  samrena_destroy(arena);
  printf("PASS: Basic generation test\n");
}

static void test_samrng_reproducibility(void) {
  Samrena *arena1 = samrena_create_default();
  Samrena *arena2 = samrena_create_default();

  SamRng *rng1 = samrng_create(arena1, 123);
  SamRng *rng2 = samrng_create(arena2, 123);

  for (int i = 0; i < 10; i++) {
    uint32_t val1 = samrng_uint32(rng1);
    uint32_t val2 = samrng_uint32(rng2);
    assert(val1 == val2);
  }

  samrng_destroy(rng1);
  samrng_destroy(rng2);
  samrena_destroy(arena1);
  samrena_destroy(arena2);
  printf("PASS: Reproducibility test\n");
}

static void test_samrng_uniform_range(void) {
  Samrena *arena = samrena_create_default();
  SamRng *rng = samrng_create(arena, 999);

  float min = -5.0f, max = 10.0f;
  for (int i = 0; i < 100; i++) {
    float val = samrng_uniform(rng, min, max);
    assert(val >= min && val <= max);
  }

  double dmin = -2.0, dmax = 7.0;
  for (int i = 0; i < 100; i++) {
    double val = samrng_uniform_double(rng, dmin, dmax);
    assert(val >= dmin && val <= dmax);
  }

  samrng_destroy(rng);
  samrena_destroy(arena);
  printf("PASS: Uniform range test\n");
}

static void test_samrng_neural_network_functions(void) {
  Samrena *arena = samrena_create_default();
  SamRng *rng = samrng_create(arena, 777);

  float xavier = samrng_xavier_uniform(rng, 100, 50);
  assert(xavier >= -sqrt(6.0f / 150.0f) && xavier <= sqrt(6.0f / 150.0f));

  float he_uniform = samrng_he_uniform(rng, 100);
  assert(he_uniform >= -sqrt(6.0f / 100.0f) && he_uniform <= sqrt(6.0f / 100.0f));

  float he_normal = samrng_he_normal(rng, 100);
  (void)he_normal;

  samrng_destroy(rng);
  samrena_destroy(arena);
  printf("PASS: Neural network functions test\n");
}

static void test_samrng_fill_functions(void) {
  Samrena *arena = samrena_create_default();
  SamRng *rng = samrng_create(arena, 888);

  float array[100];

  samrng_fill_uniform(rng, array, 100, -1.0f, 1.0f);
  for (int i = 0; i < 100; i++) {
    assert(array[i] >= -1.0f && array[i] <= 1.0f);
  }

  samrng_fill_normal(rng, array, 100, 0.0f, 1.0f);

  samrng_fill_xavier_uniform(rng, array, 100, 50, 25);
  float expected_limit = sqrt(6.0f / 75.0f);
  for (int i = 0; i < 100; i++) {
    assert(array[i] >= -expected_limit && array[i] <= expected_limit);
  }

  samrng_fill_he_uniform(rng, array, 100, 50);
  expected_limit = sqrt(6.0f / 50.0f);
  for (int i = 0; i < 100; i++) {
    assert(array[i] >= -expected_limit && array[i] <= expected_limit);
  }

  samrng_fill_he_normal(rng, array, 100, 50);

  samrng_destroy(rng);
  samrena_destroy(arena);
  printf("PASS: Fill functions test\n");
}

static void test_samrng_normal_distribution(void) {
  Samrena *arena = samrena_create_default();
  SamRng *rng = samrng_create(arena, 555);

  float mean = 5.0f, stddev = 2.0f;
  float sum = 0.0f;
  int count = 1000;

  for (int i = 0; i < count; i++) {
    float val = samrng_normal(rng, mean, stddev);
    sum += val;
  }

  float observed_mean = sum / count;
  assert(fabs(observed_mean - mean) < 0.5f);

  samrng_destroy(rng);
  samrena_destroy(arena);
  printf("PASS: Normal distribution test\n");
}

int main(void) {
  printf("Running SamRng tests...\n");

  test_samrng_creation();
  test_samrng_basic_generation();
  test_samrng_reproducibility();
  test_samrng_uniform_range();
  test_samrng_neural_network_functions();
  test_samrng_fill_functions();
  test_samrng_normal_distribution();

  printf("All SamRng tests passed!\n");
  return 0;
}