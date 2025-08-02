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
#include "samrena.h"
#include "samvector.h"

SAMRENA_DECLARE_VECTOR(int)

void double_int_transform(const void* src, void* dst, void* user_data) {
    const int* input = (const int*)src;
    int* output = (int*)dst;
    *output = (*input) * 2;
}

void print_int(const void* element, void* user_data) {
    const int* value = (const int*)element;
    const char* prefix = (const char*)user_data;
    printf("%s%d ", prefix, *value);
}

bool is_even(const void* element, void* user_data) {
    const int* value = (const int*)element;
    return (*value % 2) == 0;
}

bool is_greater_than_threshold(const void* element, void* user_data) {
    const int* value = (const int*)element;
    const int* threshold = (const int*)user_data;
    return *value > *threshold;
}

int main() {
    printf("=== SamVector Iterator and Functional Programming Examples ===\n\n");
    
    Samrena* arena = samrena_create_default();
    if (!arena) {
        fprintf(stderr, "Failed to initialize arena\n");
        return 1;
    }

    printf("1. Basic Iterator Example:\n");
    SamrenaVector_int* numbers = samrena_vector_int_init(arena, 10);
    if (!numbers) {
        fprintf(stderr, "Failed to create int vector\n");
        samrena_destroy(arena);
        return 1;
    }

    int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 0; i < 10; i++) {
        samrena_vector_int_push(numbers, &values[i]);
    }

    printf("   Original values: ");
    SamrenaVectorIterator iter = samrena_vector_iter_begin(numbers->_vec);
    while (samrena_vector_iter_has_next(&iter)) {
        const int* value = (const int*)samrena_vector_iter_next(&iter);
        if (value) {
            printf("%d ", *value);
        }
    }
    printf("\n");

    printf("\n2. Transform Example (Doubling Values):\n");
    SamrenaVector* doubled_vec = samrena_vector_map(numbers->_vec, sizeof(int), 
                                                   double_int_transform, NULL, arena);
    if (doubled_vec) {
        printf("   Doubled values: ");
        SamrenaVectorIterator doubled_iter = samrena_vector_iter_begin(doubled_vec);
        while (samrena_vector_iter_has_next(&doubled_iter)) {
            const int* value = (const int*)samrena_vector_iter_next(&doubled_iter);
            if (value) {
                printf("%d ", *value);
            }
        }
        printf("\n");
    }

    printf("\n3. ForEach Example:\n");
    printf("   Using foreach with prefix: ");
    const char* prefix = "num=";
    samrena_vector_foreach(numbers->_vec, print_int, (void*)prefix);
    printf("\n");

    printf("\n4. Filter Example (Even Numbers):\n");
    SamrenaVector* even_vec = samrena_vector_filter(numbers->_vec, is_even, NULL, arena);
    if (even_vec) {
        printf("   Even numbers: ");
        SamrenaVectorIterator even_iter = samrena_vector_iter_begin(even_vec);
        while (samrena_vector_iter_has_next(&even_iter)) {
            const int* value = (const int*)samrena_vector_iter_next(&even_iter);
            if (value) {
                printf("%d ", *value);
            }
        }
        printf("\n");
    }

    printf("\n5. Filter with User Data Example (> 5):\n");
    int threshold = 5;
    SamrenaVector* filtered_vec = samrena_vector_filter(numbers->_vec, 
                                                       is_greater_than_threshold, 
                                                       &threshold, arena);
    if (filtered_vec) {
        printf("   Numbers > %d: ", threshold);
        SamrenaVectorIterator filtered_iter = samrena_vector_iter_begin(filtered_vec);
        while (samrena_vector_iter_has_next(&filtered_iter)) {
            const int* value = (const int*)samrena_vector_iter_next(&filtered_iter);
            if (value) {
                printf("%d ", *value);
            }
        }
        printf("\n");
    }

    printf("\n6. Iterator Reset Example:\n");
    printf("   First iteration: ");
    samrena_vector_iter_reset(&iter);
    int count = 0;
    while (samrena_vector_iter_has_next(&iter) && count < 3) {
        const int* value = (const int*)samrena_vector_iter_next(&iter);
        if (value) {
            printf("%d ", *value);
            count++;
        }
    }
    
    printf("\n   Reset and full iteration: ");
    samrena_vector_iter_reset(&iter);
    while (samrena_vector_iter_has_next(&iter)) {
        const int* value = (const int*)samrena_vector_iter_next(&iter);
        if (value) {
            printf("%d ", *value);
        }
    }
    printf("\n");

    printf("\n7. Chained Operations Example:\n");
    printf("   Original -> Filter (even) -> Transform (double):\n");
    if (even_vec) {
        SamrenaVector* chained_vec = samrena_vector_map(even_vec, sizeof(int), 
                                                       double_int_transform, NULL, arena);
        if (chained_vec) {
            printf("   Result: ");
            SamrenaVectorIterator chained_iter = samrena_vector_iter_begin(chained_vec);
            while (samrena_vector_iter_has_next(&chained_iter)) {
                const int* value = (const int*)samrena_vector_iter_next(&chained_iter);
                if (value) {
                    printf("%d ", *value);
                }
            }
            printf("\n");
        }
    }

    samrena_destroy(arena);
    printf("\nIterator and functional programming example completed successfully!\n");
    return 0;
}