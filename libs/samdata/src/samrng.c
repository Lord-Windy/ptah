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

#include "samdata/samrng.h"
#include <math.h>
#include <string.h>

struct SamRng {
    uint64_t state[4];
    Samrena* arena;
    bool has_spare_normal;
    double spare_normal;
};

static uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t xoshiro256ss_next(SamRng* rng) {
    const uint64_t result = rotl(rng->state[1] * 5, 7) * 9;
    const uint64_t t = rng->state[1] << 17;

    rng->state[2] ^= rng->state[0];
    rng->state[3] ^= rng->state[1];
    rng->state[1] ^= rng->state[2];
    rng->state[0] ^= rng->state[3];

    rng->state[2] ^= t;
    rng->state[3] = rotl(rng->state[3], 45);

    return result;
}

static uint64_t splitmix64(uint64_t* state) {
    uint64_t z = (*state += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

SamRng* samrng_create(Samrena* arena, uint64_t seed) {
    if (!arena) return NULL;
    
    SamRng* rng = (SamRng*)samrena_push(arena, sizeof(SamRng));
    if (!rng) return NULL;
    
    rng->arena = arena;
    rng->has_spare_normal = false;
    rng->spare_normal = 0.0;
    
    samrng_seed(rng, seed);
    
    return rng;
}

void samrng_destroy(SamRng* rng) {
    (void)rng;
}

void samrng_seed(SamRng* rng, uint64_t seed) {
    if (!rng) return;
    
    uint64_t splitmix_state = seed;
    rng->state[0] = splitmix64(&splitmix_state);
    rng->state[1] = splitmix64(&splitmix_state);
    rng->state[2] = splitmix64(&splitmix_state);
    rng->state[3] = splitmix64(&splitmix_state);
    
    rng->has_spare_normal = false;
}

uint32_t samrng_uint32(SamRng* rng) {
    if (!rng) return 0;
    return (uint32_t)(xoshiro256ss_next(rng) >> 32);
}

uint64_t samrng_uint64(SamRng* rng) {
    if (!rng) return 0;
    return xoshiro256ss_next(rng);
}

float samrng_float(SamRng* rng) {
    if (!rng) return 0.0f;
    return (float)(xoshiro256ss_next(rng) >> 11) * 0x1.0p-53f;
}

double samrng_double(SamRng* rng) {
    if (!rng) return 0.0;
    return (double)(xoshiro256ss_next(rng) >> 11) * 0x1.0p-53;
}

float samrng_uniform(SamRng* rng, float min, float max) {
    if (!rng || min >= max) return min;
    return min + samrng_float(rng) * (max - min);
}

double samrng_uniform_double(SamRng* rng, double min, double max) {
    if (!rng || min >= max) return min;
    return min + samrng_double(rng) * (max - min);
}

float samrng_normal(SamRng* rng, float mean, float stddev) {
    if (!rng) return mean;
    
    if (rng->has_spare_normal) {
        rng->has_spare_normal = false;
        return (float)(mean + stddev * rng->spare_normal);
    }
    
    double u, v, s;
    do {
        u = 2.0 * samrng_double(rng) - 1.0;
        v = 2.0 * samrng_double(rng) - 1.0;
        s = u * u + v * v;
    } while (s >= 1.0 || s == 0.0);
    
    double multiplier = sqrt(-2.0 * log(s) / s);
    rng->spare_normal = v * multiplier;
    rng->has_spare_normal = true;
    
    return (float)(mean + stddev * u * multiplier);
}

double samrng_normal_double(SamRng* rng, double mean, double stddev) {
    if (!rng) return mean;
    
    if (rng->has_spare_normal) {
        rng->has_spare_normal = false;
        return mean + stddev * rng->spare_normal;
    }
    
    double u, v, s;
    do {
        u = 2.0 * samrng_double(rng) - 1.0;
        v = 2.0 * samrng_double(rng) - 1.0;
        s = u * u + v * v;
    } while (s >= 1.0 || s == 0.0);
    
    double multiplier = sqrt(-2.0 * log(s) / s);
    rng->spare_normal = v * multiplier;
    rng->has_spare_normal = true;
    
    return mean + stddev * u * multiplier;
}

float samrng_xavier_uniform(SamRng* rng, size_t fan_in, size_t fan_out) {
    if (!rng || fan_in == 0 || fan_out == 0) return 0.0f;
    float limit = sqrtf(6.0f / (float)(fan_in + fan_out));
    return samrng_uniform(rng, -limit, limit);
}

float samrng_he_uniform(SamRng* rng, size_t fan_in) {
    if (!rng || fan_in == 0) return 0.0f;
    float limit = sqrtf(6.0f / (float)fan_in);
    return samrng_uniform(rng, -limit, limit);
}

float samrng_he_normal(SamRng* rng, size_t fan_in) {
    if (!rng || fan_in == 0) return 0.0f;
    float stddev = sqrtf(2.0f / (float)fan_in);
    return samrng_normal(rng, 0.0f, stddev);
}

void samrng_fill_uniform(SamRng* rng, float* array, size_t count, float min, float max) {
    if (!rng || !array) return;
    for (size_t i = 0; i < count; i++) {
        array[i] = samrng_uniform(rng, min, max);
    }
}

void samrng_fill_normal(SamRng* rng, float* array, size_t count, float mean, float stddev) {
    if (!rng || !array) return;
    for (size_t i = 0; i < count; i++) {
        array[i] = samrng_normal(rng, mean, stddev);
    }
}

void samrng_fill_xavier_uniform(SamRng* rng, float* array, size_t count, size_t fan_in, size_t fan_out) {
    if (!rng || !array) return;
    for (size_t i = 0; i < count; i++) {
        array[i] = samrng_xavier_uniform(rng, fan_in, fan_out);
    }
}

void samrng_fill_he_uniform(SamRng* rng, float* array, size_t count, size_t fan_in) {
    if (!rng || !array) return;
    for (size_t i = 0; i < count; i++) {
        array[i] = samrng_he_uniform(rng, fan_in);
    }
}

void samrng_fill_he_normal(SamRng* rng, float* array, size_t count, size_t fan_in) {
    if (!rng || !array) return;
    for (size_t i = 0; i < count; i++) {
        array[i] = samrng_he_normal(rng, fan_in);
    }
}