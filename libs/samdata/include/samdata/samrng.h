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

#ifndef SAMDATA_SAMRNG_H
#define SAMDATA_SAMRNG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <samrena.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SamRng SamRng;

SamRng* samrng_create(Samrena* arena, uint64_t seed);

void samrng_destroy(SamRng* rng);

void samrng_seed(SamRng* rng, uint64_t seed);

uint32_t samrng_uint32(SamRng* rng);

uint64_t samrng_uint64(SamRng* rng);

float samrng_float(SamRng* rng);

double samrng_double(SamRng* rng);

float samrng_uniform(SamRng* rng, float min, float max);

double samrng_uniform_double(SamRng* rng, double min, double max);

float samrng_normal(SamRng* rng, float mean, float stddev);

double samrng_normal_double(SamRng* rng, double mean, double stddev);

float samrng_xavier_uniform(SamRng* rng, size_t fan_in, size_t fan_out);

float samrng_he_uniform(SamRng* rng, size_t fan_in);

float samrng_he_normal(SamRng* rng, size_t fan_in);

void samrng_fill_uniform(SamRng* rng, float* array, size_t count, float min, float max);

void samrng_fill_normal(SamRng* rng, float* array, size_t count, float mean, float stddev);

void samrng_fill_xavier_uniform(SamRng* rng, float* array, size_t count, size_t fan_in, size_t fan_out);

void samrng_fill_he_uniform(SamRng* rng, float* array, size_t count, size_t fan_in);

void samrng_fill_he_normal(SamRng* rng, float* array, size_t count, size_t fan_in);

#ifdef __cplusplus
}
#endif

#endif /* SAMDATA_SAMRNG_H */