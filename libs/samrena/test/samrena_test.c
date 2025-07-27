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
#include <assert.h>
#include <stdio.h>

#include <string.h>

char *global_dictionary[50] = {
    "ability",  "about",    "above",    "accept",    "according", "account",
    "across",   "action",   "activity", "actually",  "address",   "administration",
    "admit",    "adult",    "affect",   "after",     "again",     "against",
    "age",      "agency",   "agent",    "agreement", "ahead",     "air",
    "all",      "allow",    "almost",   "alone",     "along",     "already",
    "also",     "although", "always",   "American",  "among",     "amount",
    "analysis", "and",      "animal",   "another",   "answer",    "any",
    "anyone",   "anything", "appear",   "apply",     "approach",  "area",
    "argue",    "arm"};

void print_samrena(Samrena *samrena) {

    printf("Arena allocated: %lu\n", samrena->allocated);
    printf("Arena capacity: %lu\n", samrena->capacity);
}

void create_new_arena() {
    Samrena *samrena = samrena_allocate(10);

    print_samrena(samrena);

    int32_t *data = samrena_push_zero(samrena, 400 * sizeof(int32_t));

    printf("Address of Data %p\n", (void *)data);
    printf("Address of Samrena? %p\n", (void *)samrena);
    printf("Address of Samrena + sizeof %p\n", (void *)samrena + sizeof(Samrena));

    assert(data != 0);
    assert((void *)data == (void *)samrena + sizeof(Samrena));

    print_samrena(samrena);
    samrena_deallocate(samrena);
}

void create_multiple_arrays() {
    Samrena *samrena = samrena_allocate(10);

    int32_t *data_holder[10];

    for (int i = 0; i < 10; i++) {
        data_holder[i] = samrena_push_zero(samrena, 30 * sizeof(int32_t));
        for (int j = 0; j < 30; j++) {
            data_holder[i][j] = i * 30 + j;
            printf("data_holder[%d][%d] = %d\n", i, j, data_holder[i][j]);
        }
    }

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 30; j++) {
            printf("data_holder[%d][%d] = %d\n", i, j, data_holder[i][j]);
            assert(data_holder[i][j] == i * 30 + j);
        }
    }

    print_samrena(samrena);
    samrena_deallocate(samrena);
}

void create_multiple_strings() {
    Samrena *samrena = samrena_allocate(400);

    char **string_holder[10];

    for (int i = 0; i < 10; i++) {
        string_holder[i] = samrena_push(samrena, sizeof(char *) * 50);
        for (int j = 0; j < 50; j++) {
            char *new_string =
                samrena_push_zero(samrena, sizeof(char) * strlen(global_dictionary[j]) + 1);
            strcpy(new_string, global_dictionary[j]);
            string_holder[i][j] = new_string;
            printf("string_holder[%d][%d] = %s\n", i, j, string_holder[i][j]);
        }
    }

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 50; j++) {
            printf("string_holder[%d][%d] = %s\n", i, j, string_holder[i][j]);
            assert(strcmp(string_holder[i][j], global_dictionary[j]) == 0);
        }
    }

    print_samrena(samrena);
    samrena_deallocate(samrena);
}

// Test zero page count allocation
void test_zero_page_allocation() {
    printf("\n--- Testing zero page allocation ---\n");
    Samrena *samrena = samrena_allocate(0);

    print_samrena(samrena);

    // Try to push some data
    void *data = samrena_push(samrena, 10);
    printf("Push result with zero capacity: %p\n", data);
    assert(data == 0); // Should fail to allocate

    samrena_deallocate(samrena);
}

// Test allocation at capacity boundary
void test_capacity_boundary() {
    printf("\n--- Testing capacity boundary ---\n");
    Samrena *samrena = samrena_allocate(1); // Single page
    print_samrena(samrena);

    uint64_t remaining = samrena->capacity - samrena->allocated;
    printf("Remaining space: %lu bytes\n", remaining);

    // Allocate exactly the remaining space
    void *data1 = samrena_push(samrena, remaining);
    assert(data1 != 0);
    printf("Allocated exact remaining space: %p\n", data1);
    print_samrena(samrena);

    // Try to allocate 1 more byte (should fail)
    void *data2 = samrena_push(samrena, 1);
    assert(data2 == 0);
    printf("Allocation beyond capacity: %p\n", data2);

    samrena_deallocate(samrena);
}

// Test alignment issues with different data types
void test_data_alignment() {
    printf("\n--- Testing data alignment ---\n");
    Samrena *samrena = samrena_allocate(1);

    // Push data of different types to test alignment
    char *c = samrena_push(samrena, sizeof(char));
    printf("Char address: %p\n", (void *)c);

    // This may cause alignment issues on some platforms
    int64_t *i64 = samrena_push(samrena, sizeof(int64_t));
    printf("Int64 address: %p\n", (void *)i64);
    assert(i64 != 0);

    // Try to write to the allocated memory
    *c = 'A';
    *i64 = 0x1234567890ABCDEF;

    printf("Char value: %c\n", *c);
    printf("Int64 value: %lx\n", *i64);

    samrena_deallocate(samrena);
}

// Test extremely large allocation
void test_large_allocation() {
    printf("\n--- Testing large allocation ---\n");
    // Large but should be valid on most systems
    Samrena *samrena = samrena_allocate(1024); // 4MB

    print_samrena(samrena);

    // Allocate a large chunk but still within capacity
    void *large_data = samrena_push_zero(samrena, 1024 * 1024 * 2); // 2MB
    assert(large_data != 0);

    print_samrena(samrena);

    // Try to access the memory to ensure it's usable
    uint8_t *bytes = (uint8_t *)large_data;
    bytes[0] = 42;
    bytes[1024 * 1024] = 43;         // Middle of allocation
    bytes[2 * 1024 * 1024 - 1] = 44; // End of allocation

    assert(bytes[0] == 42);
    assert(bytes[1024 * 1024] == 43);
    assert(bytes[2 * 1024 * 1024 - 1] == 44);

    samrena_deallocate(samrena);
}

// Test minimal size allocation
void test_minimal_allocation() {
    printf("\n--- Testing minimal allocations ---\n");
    Samrena *samrena = samrena_allocate(1);

    // Test with many small allocations
    void *pointers[1000];
    int count = 0;

    for (int i = 0; i < 1000; i++) {
        pointers[i] = samrena_push(samrena, 1); // Single byte allocations
        if (pointers[i] == 0) {
            break;
        }
        count++;

        // Write to each allocated byte to ensure it's valid
        *((uint8_t *)pointers[i]) = i % 256;
    }

    printf("Successfully allocated %d individual bytes\n", count);

    // Verify data
    for (int i = 0; i < count; i++) {
        assert(*((uint8_t *)pointers[i]) == i % 256);
    }

    print_samrena(samrena);
    samrena_deallocate(samrena);
}

int main(int argc, char **argv) {

    printf("START SAMRENA TESTING\n");

    int x = 42;
    int *ptr = &x;

    printf("Address of X %p\n", &x);
    printf("pointer? %p\n", (void *)ptr);

    create_new_arena();
    create_multiple_arrays();
    create_multiple_strings();

    // Run the new corner case tests
    test_zero_page_allocation();
    test_capacity_boundary();
    test_data_alignment();
    test_large_allocation();
    test_minimal_allocation();

    printf("\nAll tests completed successfully!\n");
    return 0;
}