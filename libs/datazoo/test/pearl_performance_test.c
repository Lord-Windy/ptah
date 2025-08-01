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

#define _POSIX_C_SOURCE 200112L
#include <datazoo/pearl.h>
#include <samrena.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

static void test_pearl_large_scale_operations(void) {
    printf("Testing large scale operations...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 1024, arena);
    assert(pearl != NULL);
    
    const int num_elements = 10000;
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < num_elements; i++) {
        bool result = pearl_add(pearl, &i);
        assert(result == true);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double add_time = get_time_diff(start, end);
    
    assert(pearl_size(pearl) == (size_t)num_elements);
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < num_elements; i++) {
        bool result = pearl_contains(pearl, &i);
        assert(result == true);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double lookup_time = get_time_diff(start, end);
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < num_elements / 2; i++) {
        bool result = pearl_remove(pearl, &i);
        assert(result == true);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double remove_time = get_time_diff(start, end);
    
    assert(pearl_size(pearl) == (size_t)(num_elements / 2));
    
    printf("  Performance results for %d elements:\n", num_elements);
    printf("    Add time: %.6f seconds (%.1f ops/sec)\n", add_time, num_elements / add_time);
    printf("    Lookup time: %.6f seconds (%.1f ops/sec)\n", lookup_time, num_elements / lookup_time);
    printf("    Remove time: %.6f seconds (%.1f ops/sec)\n", remove_time, (num_elements / 2) / remove_time);
    
    PearlStats stats = pearl_get_stats(pearl);
    printf("    Total collisions: %zu\n", stats.total_collisions);
    printf("    Max chain length: %zu\n", stats.max_chain_length);
    printf("    Average chain length: %.2f\n", stats.average_chain_length);
    printf("    Resize count: %zu\n", stats.resize_count);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Large scale operations test passed\n");
}

static void test_pearl_hash_function_performance(void) {
    printf("Testing hash function performance comparison...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    PearlHashFunction functions[] = {PEARL_HASH_DJB2, PEARL_HASH_FNV1A, PEARL_HASH_MURMUR3};
    const char *function_names[] = {"DJB2", "FNV1A", "MURMUR3"};
    int num_functions = sizeof(functions) / sizeof(functions[0]);
    
    const int num_elements = 5000;
    
    for (int f = 0; f < num_functions; f++) {
        Pearl *pearl = pearl_create_with_hash(sizeof(int), 512, arena, functions[f]);
        assert(pearl != NULL);
        
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        for (int i = 0; i < num_elements; i++) {
            pearl_add(pearl, &i);
        }
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        double add_time = get_time_diff(start, end);
        
        PearlStats stats = pearl_get_stats(pearl);
        
        printf("  %s hash function:\n", function_names[f]);
        printf("    Add time: %.6f seconds\n", add_time);
        printf("    Collisions: %zu\n", stats.total_collisions);
        printf("    Max chain length: %zu\n", stats.max_chain_length);
        printf("    Average chain length: %.2f\n", stats.average_chain_length);
        
        pearl_destroy(pearl);
    }
    
    samrena_destroy(arena);
    printf("✓ Hash function performance test completed\n");
}

static uint32_t bad_hash(const void *element, size_t size) {
    (void)element;
    (void)size;
    return 1;
}

static void test_pearl_collision_heavy_scenario(void) {
    printf("Testing collision-heavy scenario...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create_custom(sizeof(int), 16, arena, bad_hash, NULL);
    assert(pearl != NULL);
    
    const int num_elements = 1000;
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < num_elements; i++) {
        bool result = pearl_add(pearl, &i);
        assert(result == true);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double add_time = get_time_diff(start, end);
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < num_elements; i++) {
        bool result = pearl_contains(pearl, &i);
        assert(result == true);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double lookup_time = get_time_diff(start, end);
    
    PearlStats stats = pearl_get_stats(pearl);
    
    printf("  Collision-heavy scenario results:\n");
    printf("    Add time: %.6f seconds\n", add_time);
    printf("    Lookup time: %.6f seconds\n", lookup_time);
    printf("    Total collisions: %zu\n", stats.total_collisions);
    printf("    Max chain length: %zu\n", stats.max_chain_length);
    printf("    Average chain length: %.2f\n", stats.average_chain_length);
    
    assert(stats.total_collisions > 0);
    assert(stats.max_chain_length > 1);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Collision-heavy scenario test passed\n");
}

static void test_pearl_resize_performance(void) {
    printf("Testing resize performance...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 4, arena);
    assert(pearl != NULL);
    
    const int num_elements = 2000;
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < num_elements; i++) {
        pearl_add(pearl, &i);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double total_time = get_time_diff(start, end);
    
    PearlStats stats = pearl_get_stats(pearl);
    
    printf("  Resize performance results:\n");
    printf("    Total time for %d additions: %.6f seconds\n", num_elements, total_time);
    printf("    Average time per operation: %.9f seconds\n", total_time / num_elements);
    printf("    Number of resizes: %zu\n", stats.resize_count);
    printf("    Final capacity growth factor: ~%.1fx\n", 
           (double)num_elements / 4.0);
    
    for (int i = 0; i < num_elements; i++) {
        assert(pearl_contains(pearl, &i) == true);
    }
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Resize performance test passed\n");
}

static void test_pearl_memory_efficiency(void) {
    printf("Testing memory efficiency...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    size_t initial_allocated = samrena_allocated(arena);
    
    Pearl *pearl = pearl_create(sizeof(int), 1024, arena);
    assert(pearl != NULL);
    
    size_t after_creation = samrena_allocated(arena);
    
    const int num_elements = 10000;
    
    for (int i = 0; i < num_elements; i++) {
        pearl_add(pearl, &i);
    }
    
    size_t after_additions = samrena_allocated(arena);
    
    printf("  Memory usage:\n");
    printf("    Initial: %zu bytes\n", initial_allocated);
    printf("    After creation: %zu bytes (+%zu)\n", after_creation, after_creation - initial_allocated);
    printf("    After %d additions: %zu bytes (+%zu)\n", 
           num_elements, after_additions, after_additions - after_creation);
    printf("    Bytes per element: %.1f\n", 
           (double)(after_additions - after_creation) / num_elements);
    
    PearlStats stats = pearl_get_stats(pearl);
    printf("    Failed allocations: %zu\n", stats.failed_allocations);
    
    assert(stats.failed_allocations == 0);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Memory efficiency test passed\n");
}

int main(void) {
    printf("=== Pearl Performance Tests ===\n");
    
    test_pearl_large_scale_operations();
    test_pearl_hash_function_performance();
    test_pearl_collision_heavy_scenario();
    test_pearl_resize_performance();
    test_pearl_memory_efficiency();
    
    printf("\n✅ All Pearl performance tests completed!\n");
    return 0;
}