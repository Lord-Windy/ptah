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
#include <string.h>
#include "samrena.h"
#include "datazoo/pearl.h"

typedef struct {
    int id;
    char name[32];
    float score;
} Student;

void print_int(const void* element, void* user_data) {
    const int* value = (const int*)element;
    const char* prefix = (const char*)user_data;
    printf("%s%d ", prefix, *value);
}

void print_student(const void* element, void* user_data) {
    const Student* student = (const Student*)element;
    printf("  ID: %d, Name: %s, Score: %.1f\n", student->id, student->name, student->score);
}

bool is_even(const void* element, void* user_data) {
    const int* value = (const int*)element;
    return (*value % 2) == 0;
}

bool high_scorer(const void* element, void* user_data) {
    const Student* student = (const Student*)element;
    const float* threshold = (const float*)user_data;
    return student->score > *threshold;
}

void double_transform(const void* src, void* dst, void* user_data) {
    const int* input = (const int*)src;
    int* output = (int*)dst;
    *output = (*input) * 2;
}

uint32_t student_hash(const void* element, size_t size) {
    const Student* student = (const Student*)element;
    return (uint32_t)student->id;
}

bool student_equals(const void* a, const void* b, size_t size) {
    const Student* s1 = (const Student*)a;
    const Student* s2 = (const Student*)b;
    return s1->id == s2->id;
}

int main() {
    printf("=== Pearl Set Data Structure Examples ===\n\n");
    
    Samrena* arena = samrena_create_default();
    if (!arena) {
        fprintf(stderr, "Failed to initialize arena\n");
        return 1;
    }

    printf("1. Basic Integer Set Operations:\n");
    Pearl* int_set = pearl_create(sizeof(int), 8, arena);
    if (!int_set) {
        fprintf(stderr, "Failed to create integer set\n");
        samrena_destroy(arena);
        return 1;
    }

    int values[] = {1, 2, 3, 4, 5, 3, 2, 6, 7, 8, 1};
    int num_values = sizeof(values) / sizeof(values[0]);

    printf("   Adding values (duplicates should be ignored): ");
    for (int i = 0; i < num_values; i++) {
        printf("%d ", values[i]);
        pearl_add(int_set, &values[i]);
    }
    printf("\n");

    printf("   Set size: %zu\n", pearl_size(int_set));
    printf("   Set contents: ");
    pearl_foreach(int_set, print_int, (void*)"");
    printf("\n");

    printf("   Contains 3: %s\n", pearl_contains(int_set, &(int){3}) ? "Yes" : "No");
    printf("   Contains 9: %s\n", pearl_contains(int_set, &(int){9}) ? "Yes" : "No");

    printf("\n2. Type-safe Integer Set (using macros):\n");
    int_set_pearl* typed_int_set = int_set_create(8, arena);
    if (!typed_int_set) {
        fprintf(stderr, "Failed to create typed integer set\n");
        samrena_destroy(arena);
        return 1;
    }

    printf("   Adding values with type-safe API: ");
    for (int i = 0; i < 5; i++) {
        int val = (i + 1) * 10;
        printf("%d ", val);
        int_set_add(typed_int_set, val);
    }
    printf("\n");

    printf("   Typed set size: %zu\n", int_set_size(typed_int_set));
    printf("   Contains 30: %s\n", int_set_contains(typed_int_set, 30) ? "Yes" : "No");

    printf("\n3. Student Set with Custom Hash Function:\n");
    Pearl* student_set = pearl_create_custom(sizeof(Student), 8, arena, 
                                           student_hash, student_equals);
    if (!student_set) {
        fprintf(stderr, "Failed to create student set\n");
        samrena_destroy(arena);
        return 1;
    }

    Student students[] = {
        {1, "Alice Johnson", 95.5f},
        {2, "Bob Smith", 87.2f},
        {3, "Charlie Brown", 92.8f},
        {1, "Alice Clone", 90.0f},  // Same ID, should be rejected
        {4, "Diana Prince", 98.7f}
    };

    printf("   Adding students (duplicate IDs ignored):\n");
    for (int i = 0; i < 5; i++) {
        bool added = pearl_add(student_set, &students[i]);
        printf("   %s: %s (ID: %d)\n", 
               students[i].name, 
               added ? "Added" : "Duplicate", 
               students[i].id);
    }

    printf("\n   Final student set:\n");
    pearl_foreach(student_set, print_student, NULL);

    printf("\n4. Set Operations and Filtering:\n");
    printf("   Removing element 2 from integer set: %s\n", 
           pearl_remove(int_set, &(int){2}) ? "Success" : "Not found");
    printf("   Set after removal: ");
    pearl_foreach(int_set, print_int, (void*)"");
    printf("\n");

    Pearl* even_set = pearl_filter(int_set, is_even, NULL, arena);
    if (even_set) {
        printf("   Even numbers only: ");
        pearl_foreach(even_set, print_int, (void*)"");
        printf("\n");
    }

    float score_threshold = 90.0f;
    Pearl* high_scorers = pearl_filter(student_set, high_scorer, &score_threshold, arena);
    if (high_scorers) {
        printf("   High scorers (> %.1f):\n", score_threshold);
        pearl_foreach(high_scorers, print_student, NULL);
    }

    printf("\n5. Set Transformation (Map):\n");
    Pearl* doubled_set = pearl_map(int_set, double_transform, sizeof(int), NULL, arena);
    if (doubled_set) {
        printf("   Original set: ");
        pearl_foreach(int_set, print_int, (void*)"");
        printf("\n   Doubled set: ");
        pearl_foreach(doubled_set, print_int, (void*)"");
        printf("\n");
    }

    printf("\n6. Array Conversion:\n");
    int array[20];
    size_t copied = pearl_to_array(int_set, array, 20);
    printf("   Copied %zu elements to array: ", copied);
    for (size_t i = 0; i < copied; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    Pearl* from_array_set = pearl_from_array(array, copied, sizeof(int), arena);
    if (from_array_set) {
        printf("   Created set from array (size: %zu): ", pearl_size(from_array_set));
        pearl_foreach(from_array_set, print_int, (void*)"");
        printf("\n");
    }

    printf("\n7. Set Copying:\n");
    Pearl* copied_set = pearl_copy(int_set, arena);
    if (copied_set) {
        printf("   Original set size: %zu\n", pearl_size(int_set));
        printf("   Copied set size: %zu\n", pearl_size(copied_set));
        printf("   Copied set contents: ");
        pearl_foreach(copied_set, print_int, (void*)"");
        printf("\n");
    }

    printf("\n8. Performance Statistics:\n");
    PearlStats stats = pearl_get_stats(int_set);
    printf("   Total operations: %zu\n", stats.total_operations);
    printf("   Total collisions: %zu\n", stats.total_collisions);
    printf("   Max chain length: %zu\n", stats.max_chain_length);
    printf("   Average chain length: %.2f\n", stats.average_chain_length);
    printf("   Resize count: %zu\n", stats.resize_count);

    printf("\n9. Error Handling:\n");
    PearlError last_error = pearl_get_last_error(int_set);
    printf("   Last error: %s\n", pearl_error_string(last_error));

    printf("\n10. Clear and Empty Check:\n");
    printf("   Before clear - Empty: %s, Size: %zu\n", 
           pearl_is_empty(int_set) ? "Yes" : "No", pearl_size(int_set));
    pearl_clear(int_set);
    printf("   After clear - Empty: %s, Size: %zu\n", 
           pearl_is_empty(int_set) ? "Yes" : "No", pearl_size(int_set));

    samrena_destroy(arena);
    printf("\nPearl set example completed successfully!\n");
    return 0;
}