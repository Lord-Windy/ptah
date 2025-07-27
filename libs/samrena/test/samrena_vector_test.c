#include "samrena.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// Test vector creation and basic operations
void test_vector_init() {
    printf("Testing vector initialization... ");
    
    Samrena* arena = samrena_allocate(10);
    SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 10);
    
    assert(vec != NULL);
    assert(vec->size == 0);
    assert(vec->element_size == sizeof(int));
    assert(vec->capacity == 10);
    assert(vec->data != NULL);
    
    samrena_deallocate(arena);
    printf("PASSED\n");
}

// Test pushing elements to the vector
void test_vector_push() {
    printf("Testing vector push... ");
    
    Samrena* arena = samrena_allocate(10);
    SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 3);
    
    int values[] = {10, 20, 30, 40, 50};
    
    // Push 3 elements
    for (int i = 0; i < 3; i++) {
        void* result = samrena_vector_push(vec, arena, &values[i]);
        assert(result != NULL);
    }
    
    assert(vec->size == 3);
    
    // Push 2 more elements (should trigger resize)
    for (int i = 3; i < 5; i++) {
        void* result = samrena_vector_push(vec, arena, &values[i]);
        assert(result != NULL);
    }
    
    assert(vec->size == 5);
    assert(vec->capacity >= 5);
    
    // Verify all elements
    for (int i = 0; i < 5; i++) {
        int* element = (int*)((uint8_t*)vec->data + (i * vec->element_size));
        assert(*element == values[i]);
    }
    
    samrena_deallocate(arena);
    printf("PASSED\n");
}

// Test popping elements from the vector
void test_vector_pop() {
    printf("Testing vector pop... ");
    
    Samrena* arena = samrena_allocate(10);
    SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 5);
    
    int values[] = {10, 20, 30, 40, 50};
    
    // Push 5 elements
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(vec, arena, &values[i]);
    }
    
    assert(vec->size == 5);
    
    // Pop elements and verify
    for (int i = 4; i >= 0; i--) {
        int* popped = (int*)samrena_vector_pop(vec);
        assert(popped != NULL);
        assert(*popped == values[i]);
        assert(vec->size == (uint64_t)i);
    }
    
    // Try popping from empty vector
    void* result = samrena_vector_pop(vec);
    assert(result == NULL);
    
    samrena_deallocate(arena);
    printf("PASSED\n");
}

// Test vector resize
void test_vector_resize() {
    printf("Testing vector resize... ");
    
    Samrena* arena = samrena_allocate(10);
    SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 5);
    
    int values[10];
    for (int i = 0; i < 10; i++) {
        values[i] = i * 10;
    }
    
    // Push 5 elements
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(vec, arena, &values[i]);
    }
    
    uint64_t original_capacity = vec->capacity;
    
    // Manually resize vector
    void* result = samrena_vector_resize(vec, arena, 2);
    assert(result != NULL);
    assert(vec->capacity == original_capacity * 2);
    
    // Verify existing elements remain intact
    for (int i = 0; i < 5; i++) {
        int* element = (int*)((uint8_t*)vec->data + (i * vec->element_size));
        assert(*element == values[i]);
    }
    
    // Push more elements to fill expanded capacity
    for (int i = 5; i < 10; i++) {
        samrena_vector_push(vec, arena, &values[i]);
    }
    
    assert(vec->size == 10);
    
    // Verify all elements
    for (int i = 0; i < 10; i++) {
        int* element = (int*)((uint8_t*)vec->data + (i * vec->element_size));
        assert(*element == values[i]);
    }
    
    samrena_deallocate(arena);
    printf("PASSED\n");
}

// Test with different data types
void test_different_types() {
    printf("Testing vector with different data types... ");
    
    Samrena* arena = samrena_allocate(10);
    
    // Test with char
    SamrenaVector* char_vec = samrena_vector_init(arena, sizeof(char), 5);
    char chars[] = {'a', 'b', 'c', 'd', 'e'};
    
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(char_vec, arena, &chars[i]);
    }
    
    for (int i = 0; i < 5; i++) {
        char* element = (char*)((uint8_t*)char_vec->data + (i * char_vec->element_size));
        assert(*element == chars[i]);
    }
    
    // Test with double
    SamrenaVector* double_vec = samrena_vector_init(arena, sizeof(double), 5);
    double doubles[] = {1.1, 2.2, 3.3, 4.4, 5.5};
    
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(double_vec, arena, &doubles[i]);
    }
    
    for (int i = 0; i < 5; i++) {
        double* element = (double*)((uint8_t*)double_vec->data + (i * double_vec->element_size));
        assert(*element == doubles[i]);
    }
    
    // Test with struct
    typedef struct {
        int id;
        char name[10];
        double value;
    } TestStruct;
    
    SamrenaVector* struct_vec = samrena_vector_init(arena, sizeof(TestStruct), 3);
    
    TestStruct structs[3];
    
    for (int i = 0; i < 3; i++) {
        structs[i].id = i + 1;
        snprintf(structs[i].name, 10, "Name%d", i + 1);
        structs[i].value = (i + 1) * 2.5;
        samrena_vector_push(struct_vec, arena, &structs[i]);
    }
    
    for (int i = 0; i < 3; i++) {
        TestStruct* element = (TestStruct*)((uint8_t*)struct_vec->data + (i * struct_vec->element_size));
        assert(element->id == structs[i].id);
        assert(strcmp(element->name, structs[i].name) == 0);
        assert(element->value == structs[i].value);
    }
    
    samrena_deallocate(arena);
    printf("PASSED\n");
}

// Test edge cases
void test_edge_cases() {
    printf("Testing vector edge cases... ");
    
    Samrena* arena = samrena_allocate(10);
    
    // Zero capacity
    SamrenaVector* vec_zero = samrena_vector_init(arena, sizeof(int), 0);
    assert(vec_zero != NULL);
    assert(vec_zero->capacity > 0); // Should use default capacity
    
    // Large number of elements
    int num_elements = 1000;
    SamrenaVector* vec_large = samrena_vector_init(arena, sizeof(int), num_elements);
    
    for (int i = 0; i < num_elements; i++) {
        samrena_vector_push(vec_large, arena, &i);
    }
    
    assert(vec_large->size == (uint64_t)num_elements);
    
    // Verify some elements
    int* first = (int*)((uint8_t*)vec_large->data);
    int* middle = (int*)((uint8_t*)vec_large->data + ((num_elements / 2) * vec_large->element_size));
    int* last = (int*)((uint8_t*)vec_large->data + ((num_elements - 1) * vec_large->element_size));
    
    assert(*first == 0);
    assert(*middle == num_elements / 2);
    assert(*last == num_elements - 1);
    
    // Test small element size
    SamrenaVector* vec_small = samrena_vector_init(arena, 1, 10); // 1-byte elements
    uint8_t small_vals[] = {0xFF, 0xAA, 0x55, 0x01};
    
    for (int i = 0; i < 4; i++) {
        samrena_vector_push(vec_small, arena, &small_vals[i]);
    }
    
    for (int i = 0; i < 4; i++) {
        uint8_t* element = (uint8_t*)vec_small->data + i;
        assert(*element == small_vals[i]);
    }
    
    samrena_deallocate(arena);
    printf("PASSED\n");
}

int main() {
    printf("Starting samrena vector tests...\n\n");
    
    test_vector_init();
    test_vector_push();
    test_vector_pop();
    test_vector_resize();
    test_different_types();
    test_edge_cases();
    
    printf("\nAll samrena vector tests passed!\n");
    return 0;
}