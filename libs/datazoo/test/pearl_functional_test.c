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

#include <datazoo/pearl.h>
#include <samrena.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

// Predicate function: filter even numbers
bool is_even(const void *element, void *user_data) {
    (void)user_data; // Unused parameter
    int value = *(const int *)element;
    return value % 2 == 0;
}

// Predicate function: filter numbers greater than threshold
bool greater_than_threshold(const void *element, void *user_data) {
    int value = *(const int *)element;
    int threshold = *(const int *)user_data;
    return value > threshold;
}

// Transform function: square numbers
void square_transform(const void *input, void *output, void *user_data) {
    (void)user_data; // Unused parameter
    int value = *(const int *)input;
    *(int *)output = value * value;
}

// Transform function: multiply by factor
void multiply_transform(const void *input, void *output, void *user_data) {
    int value = *(const int *)input;
    int factor = *(const int *)user_data;
    *(int *)output = value * factor;
}

// Complex predicate: filter strings by length
bool string_length_filter(const void *element, void *user_data) {
    const char *str = *(const char **)element;
    size_t min_length = *(const size_t *)user_data;
    return strlen(str) >= min_length;
}

// Complex transform: string to length
void string_to_length_transform(const void *input, void *output, void *user_data) {
    (void)user_data; // Unused parameter
    const char *str = *(const char **)input;
    *(size_t *)output = strlen(str);
}

void test_pearl_filter_basic() {
    printf("Testing pearl_filter with basic predicate...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *original = pearl_create(sizeof(int), 16, arena);
    assert(original != NULL);
    
    // Add test data: 1, 2, 3, 4, 5, 6
    for (int i = 1; i <= 6; i++) {
        assert(pearl_add(original, &i));
    }
    
    assert(pearl_size(original) == 6);
    
    // Filter even numbers
    Pearl *filtered = pearl_filter(original, is_even, NULL, arena);
    assert(filtered != NULL);
    assert(pearl_size(filtered) == 3); // Should contain 2, 4, 6
    
    // Verify filtered content
    for (int i = 2; i <= 6; i += 2) {
        assert(pearl_contains(filtered, &i));
    }
    
    // Verify odd numbers are not present
    for (int i = 1; i <= 5; i += 2) {
        assert(!pearl_contains(filtered, &i));
    }
    
    samrena_destroy(arena);
    printf("✓ Basic filter test passed\n");
}

void test_pearl_filter_with_user_data() {
    printf("Testing pearl_filter with user data...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *original = pearl_create(sizeof(int), 16, arena);
    assert(original != NULL);
    
    // Add test data: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    for (int i = 1; i <= 10; i++) {
        assert(pearl_add(original, &i));
    }
    
    int threshold = 5;
    Pearl *filtered = pearl_filter(original, greater_than_threshold, &threshold, arena);
    assert(filtered != NULL);
    assert(pearl_size(filtered) == 5); // Should contain 6, 7, 8, 9, 10
    
    // Verify filtered content
    for (int i = 6; i <= 10; i++) {
        assert(pearl_contains(filtered, &i));
    }
    
    samrena_destroy(arena);
    printf("✓ Filter with user data test passed\n");
}

void test_pearl_map_basic() {
    printf("Testing pearl_map with basic transform...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *original = pearl_create(sizeof(int), 16, arena);
    assert(original != NULL);
    
    // Add test data: 1, 2, 3, 4
    for (int i = 1; i <= 4; i++) {
        assert(pearl_add(original, &i));
    }
    
    // Map to squares
    Pearl *mapped = pearl_map(original, square_transform, sizeof(int), NULL, arena);
    assert(mapped != NULL);
    assert(pearl_size(mapped) == 4); // Should contain 1, 4, 9, 16
    
    // Verify mapped content
    int expected[] = {1, 4, 9, 16};
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
        assert(pearl_contains(mapped, &expected[i]));
    }
    
    samrena_destroy(arena);
    printf("✓ Basic map test passed\n");
}

void test_pearl_map_with_user_data() {
    printf("Testing pearl_map with user data...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *original = pearl_create(sizeof(int), 16, arena);
    assert(original != NULL);
    
    // Add test data: 1, 2, 3
    for (int i = 1; i <= 3; i++) {
        assert(pearl_add(original, &i));
    }
    
    int factor = 10;
    Pearl *mapped = pearl_map(original, multiply_transform, sizeof(int), &factor, arena);
    assert(mapped != NULL);
    assert(pearl_size(mapped) == 3); // Should contain 10, 20, 30
    
    // Verify mapped content
    int expected[] = {10, 20, 30};
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
        assert(pearl_contains(mapped, &expected[i]));
    }
    
    samrena_destroy(arena);
    printf("✓ Map with user data test passed\n");
}

void test_simple_map_debug() {
    printf("Testing simple map debug...\n");
    
    SamrenaConfig config = samrena_default_config();
    config.initial_pages = 10; 
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *original = pearl_create(sizeof(char *), 4, arena);
    assert(original != NULL);
    
    const char *test_str = "hello";
    assert(pearl_add(original, &test_str));
    printf("Added one string, set size: %zu\n", pearl_size(original));
    
    // Try to map to lengths
    Pearl *lengths = pearl_map(original, string_to_length_transform, sizeof(size_t), NULL, arena);
    if (lengths == NULL) {
        printf("ERROR: Map operation failed!\n");
    } else {
        printf("Map succeeded, size: %zu\n", pearl_size(lengths));
    }
    
    samrena_destroy(arena);
    printf("Debug test done\n");
}

void test_complex_predicates() {
    printf("Testing complex predicates with strings...\n");
    
    SamrenaConfig config = samrena_default_config();
    config.initial_pages = 10; // More pages for larger allocations
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *original = pearl_create(sizeof(char *), 16, arena);
    assert(original != NULL);
    
    // Add test strings
    const char *strings[] = {"a", "hello"};
    size_t num_strings = sizeof(strings) / sizeof(strings[0]);
    
    for (size_t i = 0; i < num_strings; i++) {
        assert(pearl_add(original, &strings[i]));
    }
    
    printf("Original set size: %zu\n", pearl_size(original));
    
    // Filter strings with length >= 5
    size_t min_length = 5;
    Pearl *filtered = pearl_filter(original, string_length_filter, &min_length, arena);
    assert(filtered != NULL);
    printf("Filtered set size: %zu\n", pearl_size(filtered));
    assert(pearl_size(filtered) == 1); // "hello"
    
    // Test simple map operation first
    printf("Testing simple map to lengths...\n");
    Pearl *lengths = pearl_map(original, string_to_length_transform, sizeof(size_t), NULL, arena);
    if (lengths == NULL) {
        printf("ERROR: Map operation failed!\n");
        assert(false);
    }
    printf("Mapped set size: %zu\n", pearl_size(lengths));
    assert(pearl_size(lengths) == num_strings);
    
    // Verify some lengths are present (but skip duplicates due to set nature)
    size_t expected_lengths[] = {1, 5}; // unique lengths of the strings
    for (size_t i = 0; i < sizeof(expected_lengths) / sizeof(expected_lengths[0]); i++) {
        assert(pearl_contains(lengths, &expected_lengths[i]));
    }
    
    samrena_destroy(arena);
    printf("✓ Complex predicates test passed\n");
}

void test_empty_set_operations() {
    printf("Testing functional operations on empty sets...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *empty = pearl_create(sizeof(int), 16, arena);
    assert(empty != NULL);
    assert(pearl_is_empty(empty));
    
    // Filter empty set
    Pearl *filtered = pearl_filter(empty, is_even, NULL, arena);
    assert(filtered != NULL);
    assert(pearl_is_empty(filtered));
    
    // Map empty set
    Pearl *mapped = pearl_map(empty, square_transform, sizeof(int), NULL, arena);
    assert(mapped != NULL);
    assert(pearl_is_empty(mapped));
    
    samrena_destroy(arena);
    printf("✓ Empty set operations test passed\n");
}

int main() {
    printf("Starting Pearl Functional Programming Tests\n");
    printf("==========================================\n");
    
    test_pearl_filter_basic();
    test_pearl_filter_with_user_data();
    test_pearl_map_basic();
    test_pearl_map_with_user_data();
    test_simple_map_debug();
    test_complex_predicates();
    test_empty_set_operations();
    
    printf("==========================================\n");
    printf("Basic functional programming tests passed! ✓\n");
    
    return 0;
}