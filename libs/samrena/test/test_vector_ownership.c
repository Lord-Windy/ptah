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

#include "samrena.h"
#include "samvector.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Test owned vector creation and destruction
void test_owned_vector_lifecycle() {
    printf("Testing owned vector lifecycle...\n");
    
    // Create vector with owned arena
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    assert(vec->owns_arena == true);
    assert(vec->arena != NULL);
    assert(vec->capacity >= 10);
    assert(vec->size == 0);
    
    // Push some data
    for (int i = 0; i < 20; i++) {
        void* result = samrena_vector_push_owned(vec, &i);
        assert(result != NULL);
    }
    assert(vec->size == 20);
    
    // Verify data
    for (int i = 0; i < 20; i++) {
        int* value = (int*)samrena_vector_at(vec, i);
        assert(*value == i);
    }
    
    // Destroy vector (should also destroy arena)
    samrena_vector_destroy(vec);
    
    printf("  ✓ Owned vector lifecycle test passed\n");
}

// Test shared arena vectors
void test_shared_arena_vectors() {
    printf("Testing shared arena vectors...\n");
    
    // Create shared arena
    SamrenaConfig config = samrena_default_config();
    config.initial_pages = 10;
    Samrena* shared_arena = samrena_create(&config);
    assert(shared_arena != NULL);
    
    // Create multiple vectors sharing the arena
    SamrenaVector* vec1 = samrena_vector_init_with_arena(shared_arena, sizeof(int), 5);
    SamrenaVector* vec2 = samrena_vector_init_with_arena(shared_arena, sizeof(float), 10);
    SamrenaVector* vec3 = samrena_vector_init_with_arena(shared_arena, sizeof(double), 15);
    
    assert(vec1 != NULL && vec1->owns_arena == false);
    assert(vec2 != NULL && vec2->owns_arena == false);
    assert(vec3 != NULL && vec3->owns_arena == false);
    
    // All vectors should use the same arena
    assert(vec1->arena == shared_arena);
    assert(vec2->arena == shared_arena);
    assert(vec3->arena == shared_arena);
    
    // Push data to each vector
    for (int i = 0; i < 10; i++) {
        int val1 = i;
        float val2 = i * 1.5f;
        double val3 = i * 2.5;
        
        samrena_vector_push(shared_arena, vec1, &val1);
        samrena_vector_push(shared_arena, vec2, &val2);
        samrena_vector_push(shared_arena, vec3, &val3);
    }
    
    // Destroy vectors (should NOT destroy shared arena)
    samrena_vector_destroy(vec1);
    samrena_vector_destroy(vec2);
    samrena_vector_destroy(vec3);
    
    // Arena should still be valid - create another vector to verify
    SamrenaVector* vec4 = samrena_vector_init_with_arena(shared_arena, sizeof(char), 20);
    assert(vec4 != NULL);
    
    // Clean up
    samrena_destroy(shared_arena);
    
    printf("  ✓ Shared arena vectors test passed\n");
}

// Test auto-detection functions
void test_auto_detection_functions() {
    printf("Testing auto-detection functions...\n");
    
    // Test with owned vector
    SamrenaVector* owned_vec = samrena_vector_init_owned(sizeof(int), 5);
    assert(owned_vec != NULL);
    
    // Push using auto function
    for (int i = 0; i < 10; i++) {
        void* result = samrena_vector_push_auto(owned_vec, &i);
        assert(result != NULL);
    }
    assert(owned_vec->size == 10);
    
    // Reserve using auto function
    SamrenaVectorError err = samrena_vector_reserve_auto(owned_vec, 20);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(owned_vec->capacity >= 20);
    
    // Test with shared arena vector
    SamrenaConfig config = samrena_default_config();
    Samrena* arena = samrena_create(&config);
    SamrenaVector* shared_vec = samrena_vector_init_with_arena(arena, sizeof(double), 5);
    assert(shared_vec != NULL);
    
    // Push using auto function
    for (int i = 0; i < 5; i++) {
        double val = i * 3.14;
        void* result = samrena_vector_push_auto(shared_vec, &val);
        assert(result != NULL);
    }
    assert(shared_vec->size == 5);
    
    // Clean up
    samrena_vector_destroy(owned_vec);
    samrena_vector_destroy(shared_vec);
    samrena_destroy(arena);
    
    printf("  ✓ Auto-detection functions test passed\n");
}

// Test explicit arena override
void test_explicit_arena_override() {
    printf("Testing explicit arena override...\n");
    
    // Create two arenas
    SamrenaConfig config1 = samrena_default_config();
    SamrenaConfig config2 = samrena_default_config();
    Samrena* arena1 = samrena_create(&config1);
    Samrena* arena2 = samrena_create(&config2);
    
    // Create vector with arena1
    SamrenaVector* vec = samrena_vector_init_with_arena(arena1, sizeof(int), 5);
    assert(vec != NULL);
    assert(vec->arena == arena1);
    
    // Push using arena2 explicitly
    for (int i = 0; i < 10; i++) {
        void* result = samrena_vector_push_with_arena(arena2, vec, &i);
        assert(result != NULL);
    }
    assert(vec->size == 10);
    
    // Note: This test demonstrates the API but mixing arenas is generally not recommended
    // as it can lead to memory management issues
    
    // Clean up
    samrena_vector_destroy(vec);
    samrena_destroy(arena1);
    samrena_destroy(arena2);
    
    printf("  ✓ Explicit arena override test passed\n");
}

// Test backward compatibility
void test_backward_compatibility() {
    printf("Testing backward compatibility...\n");
    
    // Create arena and vector using old API
    SamrenaConfig config = samrena_default_config();
    Samrena* arena = samrena_create(&config);
    SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 10);
    assert(vec != NULL);
    assert(vec->owns_arena == false); // Should default to shared arena model
    
    // Use old push API
    for (int i = 0; i < 5; i++) {
        void* result = samrena_vector_push(arena, vec, &i);
        assert(result != NULL);
    }
    
    // Use old resize API
    void* data = samrena_vector_resize(arena, vec, 20);
    assert(data != NULL);
    assert(vec->capacity == 20);
    
    // Clean up
    samrena_destroy(arena);
    
    printf("  ✓ Backward compatibility test passed\n");
}

// Test memory safety with owned vectors
void test_owned_vector_memory_safety() {
    printf("Testing owned vector memory safety...\n");
    
    // Create owned vector
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 5);
    assert(vec != NULL);
    
    // Fill to capacity and beyond to trigger reallocation
    for (int i = 0; i < 100; i++) {
        void* result = samrena_vector_push_owned(vec, &i);
        assert(result != NULL);
    }
    
    // Verify all data is still valid after reallocations
    for (int i = 0; i < 100; i++) {
        int* value = (int*)samrena_vector_at(vec, i);
        assert(*value == i);
    }
    
    // Test reserve on owned vector
    SamrenaVectorError err = samrena_vector_reserve_owned(vec, 200);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(vec->capacity >= 200);
    
    // Verify data integrity after reserve
    for (int i = 0; i < 100; i++) {
        int* value = (int*)samrena_vector_at(vec, i);
        assert(*value == i);
    }
    
    // Clean up
    samrena_vector_destroy(vec);
    
    printf("  ✓ Owned vector memory safety test passed\n");
}

// Test ownership query functions
void test_ownership_query_functions() {
    printf("Testing ownership query functions...\n");
    
    // Test with owned vector
    SamrenaVector* owned_vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(owned_vec != NULL);
    
    assert(samrena_vector_owns_arena(owned_vec) == true);
    assert(samrena_vector_get_arena(owned_vec) != NULL);
    assert(samrena_vector_can_grow(owned_vec) == true);
    
    // Test with shared arena vector
    SamrenaConfig config = samrena_default_config();
    Samrena* arena = samrena_create(&config);
    SamrenaVector* shared_vec = samrena_vector_init_with_arena(arena, sizeof(int), 10);
    assert(shared_vec != NULL);
    
    assert(samrena_vector_owns_arena(shared_vec) == false);
    assert(samrena_vector_get_arena(shared_vec) == arena);
    assert(samrena_vector_can_grow(shared_vec) == true);
    
    // Test with NULL vector
    assert(samrena_vector_owns_arena(NULL) == false);
    assert(samrena_vector_get_arena(NULL) == NULL);
    assert(samrena_vector_can_grow(NULL) == false);
    
    // Clean up
    samrena_vector_destroy(owned_vec);
    samrena_vector_destroy(shared_vec);
    samrena_destroy(arena);
    
    printf("  ✓ Ownership query functions test passed\n");
}

// Test migration helpers
void test_migration_helpers() {
    printf("Testing migration helpers...\n");
    
    // Create shared arena vector with data
    SamrenaConfig config = samrena_default_config();
    Samrena* arena = samrena_create(&config);
    SamrenaVector* shared_vec = samrena_vector_init_with_arena(arena, sizeof(int), 5);
    assert(shared_vec != NULL);
    
    // Add some data
    for (int i = 0; i < 10; i++) {
        samrena_vector_push_auto(shared_vec, &i);
    }
    assert(shared_vec->size == 10);
    
    // Convert to owned vector
    SamrenaVector* owned_vec = samrena_vector_make_owned(shared_vec);
    assert(owned_vec != NULL);
    assert(owned_vec->owns_arena == true);
    assert(owned_vec->size == shared_vec->size);
    assert(owned_vec->element_size == shared_vec->element_size);
    
    // Verify data was copied correctly
    for (int i = 0; i < 10; i++) {
        int* shared_val = (int*)samrena_vector_at(shared_vec, i);
        int* owned_val = (int*)samrena_vector_at(owned_vec, i);
        assert(*shared_val == *owned_val);
        assert(*owned_val == i);
    }
    
    // Test cleanup function
    samrena_vector_cleanup(shared_vec);
    assert(shared_vec->data == NULL);
    assert(shared_vec->size == 0);
    assert(shared_vec->capacity == 0);
    
    // Test make_owned with NULL and already owned vector
    assert(samrena_vector_make_owned(NULL) == NULL);
    assert(samrena_vector_make_owned(owned_vec) == NULL);
    
    // Clean up
    samrena_vector_destroy(owned_vec);
    samrena_destroy(arena);
    
    printf("  ✓ Migration helpers test passed\n");
}

// Test error conditions
void test_error_conditions() {
    printf("Testing error conditions...\n");
    
    // Test owned operations on shared vector
    SamrenaConfig config = samrena_default_config();
    Samrena* arena = samrena_create(&config);
    SamrenaVector* shared_vec = samrena_vector_init_with_arena(arena, sizeof(int), 5);
    
    int value = 42;
    assert(samrena_vector_push_owned(shared_vec, &value) == NULL);
    assert(samrena_vector_reserve_owned(shared_vec, 10) == SAMRENA_VECTOR_ERROR_INVALID_OPERATION);
    
    // Test cleanup on owned vector (should do nothing)
    SamrenaVector* owned_vec = samrena_vector_init_owned(sizeof(int), 5);
    size_t orig_size = owned_vec->size;
    samrena_vector_cleanup(owned_vec);
    assert(owned_vec->size == orig_size); // Should be unchanged
    
    // Clean up
    samrena_vector_destroy(shared_vec);
    samrena_vector_destroy(owned_vec);
    samrena_destroy(arena);
    
    printf("  ✓ Error conditions test passed\n");
}

int main() {
    printf("\n=== Samrena Vector Memory Ownership Tests ===\n\n");
    
    test_owned_vector_lifecycle();
    test_shared_arena_vectors();
    test_auto_detection_functions();
    test_explicit_arena_override();
    test_backward_compatibility();
    test_owned_vector_memory_safety();
    test_ownership_query_functions();
    test_migration_helpers();
    test_error_conditions();
    
    printf("\n✅ All memory ownership tests passed!\n\n");
    return 0;
}