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
#include <stdlib.h>
#include <samrena.h>
#include <samdata/samrng.h>

void print_separator(const char* title) {
    printf("\n=== %s ===\n", title);
}

void demo_basic_generation(SamRng* rng) {
    print_separator("Basic Random Number Generation");
    
    printf("Random uint32: %u\n", samrng_uint32(rng));
    printf("Random uint64: %llu\n", (unsigned long long)samrng_uint64(rng));
    printf("Random float [0,1): %.6f\n", samrng_float(rng));
    printf("Random double [0,1): %.10f\n", samrng_double(rng));
}

void demo_uniform_distribution(SamRng* rng) {
    print_separator("Uniform Distribution");
    
    printf("Uniform float [10.0, 20.0]: %.6f\n", samrng_uniform(rng, 10.0f, 20.0f));
    printf("Uniform float [-5.0, 5.0]: %.6f\n", samrng_uniform(rng, -5.0f, 5.0f));
    printf("Uniform double [100.0, 200.0]: %.10f\n", samrng_uniform_double(rng, 100.0, 200.0));
    
    printf("\nGenerating 10 dice rolls (1-6):\n");
    for (int i = 0; i < 10; i++) {
        int dice = (int)samrng_uniform(rng, 1.0f, 7.0f);
        printf("%d ", dice);
    }
    printf("\n");
}

void demo_normal_distribution(SamRng* rng) {
    print_separator("Normal (Gaussian) Distribution");
    
    printf("Normal float (mean=0, stddev=1): %.6f\n", samrng_normal(rng, 0.0f, 1.0f));
    printf("Normal float (mean=100, stddev=15): %.6f\n", samrng_normal(rng, 100.0f, 15.0f));
    printf("Normal double (mean=0, stddev=1): %.10f\n", samrng_normal_double(rng, 0.0, 1.0));
    
    printf("\nGenerating 10 IQ scores (mean=100, stddev=15):\n");
    for (int i = 0; i < 10; i++) {
        float iq = samrng_normal(rng, 100.0f, 15.0f);
        printf("%.1f ", iq);
    }
    printf("\n");
}

void demo_neural_network_initialization(SamRng* rng) {
    print_separator("Neural Network Weight Initialization");
    
    size_t fan_in = 128;
    size_t fan_out = 64;
    
    printf("Xavier uniform (fan_in=%zu, fan_out=%zu): %.6f\n", 
           fan_in, fan_out, samrng_xavier_uniform(rng, fan_in, fan_out));
    printf("He uniform (fan_in=%zu): %.6f\n", 
           fan_in, samrng_he_uniform(rng, fan_in));
    printf("He normal (fan_in=%zu): %.6f\n", 
           fan_in, samrng_he_normal(rng, fan_in));
    
    printf("\nGenerating Xavier uniform weights for a small layer:\n");
    for (int i = 0; i < 8; i++) {
        printf("%.6f ", samrng_xavier_uniform(rng, fan_in, fan_out));
    }
    printf("\n");
}

void demo_array_filling(SamRng* rng, Samrena* arena) {
    print_separator("Array Filling Functions");
    
    const size_t array_size = 10;
    float* array = samrena_push(arena, sizeof(float) * array_size);
    
    printf("Uniform array [0, 10]:\n");
    samrng_fill_uniform(rng, array, array_size, 0.0f, 10.0f);
    for (size_t i = 0; i < array_size; i++) {
        printf("%.2f ", array[i]);
    }
    printf("\n");
    
    printf("\nNormal array (mean=5, stddev=2):\n");
    samrng_fill_normal(rng, array, array_size, 5.0f, 2.0f);
    for (size_t i = 0; i < array_size; i++) {
        printf("%.2f ", array[i]);
    }
    printf("\n");
    
    printf("\nXavier uniform array (fan_in=100, fan_out=50):\n");
    samrng_fill_xavier_uniform(rng, array, array_size, 100, 50);
    for (size_t i = 0; i < array_size; i++) {
        printf("%.4f ", array[i]);
    }
    printf("\n");
    
    printf("\nHe uniform array (fan_in=256):\n");
    samrng_fill_he_uniform(rng, array, array_size, 256);
    for (size_t i = 0; i < array_size; i++) {
        printf("%.4f ", array[i]);
    }
    printf("\n");
    
    printf("\nHe normal array (fan_in=512):\n");
    samrng_fill_he_normal(rng, array, array_size, 512);
    for (size_t i = 0; i < array_size; i++) {
        printf("%.4f ", array[i]);
    }
    printf("\n");
}

void demo_seeding(SamRng* rng) {
    print_separator("Seeding and Reproducibility");
    
    uint64_t seed = 12345;
    printf("Setting seed to %llu\n", (unsigned long long)seed);
    samrng_seed(rng, seed);
    
    printf("First sequence:\n");
    for (int i = 0; i < 5; i++) {
        printf("%u ", samrng_uint32(rng));
    }
    printf("\n");
    
    printf("Resetting seed to %llu\n", (unsigned long long)seed);
    samrng_seed(rng, seed);
    
    printf("Second sequence (should be identical):\n");
    for (int i = 0; i < 5; i++) {
        printf("%u ", samrng_uint32(rng));
    }
    printf("\n");
}

int main() {
    printf("SamRng Example - Random Number Generation Library Demo\n");
    printf("=====================================================\n");
    
    Samrena* arena = samrena_create_default();
    if (!arena) {
        fprintf(stderr, "Failed to create arena\n");
        return 1;
    }
    
    uint64_t seed = 42;
    SamRng* rng = samrng_create(arena, seed);
    if (!rng) {
        fprintf(stderr, "Failed to create RNG\n");
        samrena_destroy(arena);
        return 1;
    }
    
    printf("Initialized SamRng with seed: %llu\n", (unsigned long long)seed);
    
    demo_basic_generation(rng);
    demo_uniform_distribution(rng);
    demo_normal_distribution(rng);
    demo_neural_network_initialization(rng);
    demo_array_filling(rng, arena);
    demo_seeding(rng);
    
    print_separator("Demo Complete");
    printf("SamRng provides:\n");
    printf("- Basic random number generation (uint32, uint64, float, double)\n");
    printf("- Uniform and normal distributions\n");
    printf("- Neural network weight initialization (Xavier, He)\n");
    printf("- Efficient array filling functions\n");
    printf("- Reproducible results with seeding\n");
    printf("- Arena-based memory management\n");
    
    samrng_destroy(rng);
    samrena_destroy(arena);
    
    return 0;
}