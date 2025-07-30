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
#include "samrena_internal.h"
#include <stdlib.h>
#include <string.h>

static SamrenaError last_error = SAMRENA_SUCCESS;

SamrenaError samrena_get_last_error(void) { 
    return last_error; 
}

const char *samrena_error_string(SamrenaError error) {
    switch (error) {
        case SAMRENA_SUCCESS:
            return "Success";
        case SAMRENA_ERROR_NULL_POINTER:
            return "Null pointer error";
        case SAMRENA_ERROR_INVALID_SIZE:
            return "Invalid size error";
        case SAMRENA_ERROR_OUT_OF_MEMORY:
            return "Out of memory error";
        case SAMRENA_ERROR_OVERFLOW:
            return "Overflow error";
        case SAMRENA_ERROR_INVALID_PARAMETER:
            return "Invalid parameter error";
        case SAMRENA_ERROR_UNSUPPORTED_STRATEGY:
            return "Unsupported strategy error";
        case SAMRENA_ERROR_UNSUPPORTED_OPERATION:
            return "Unsupported operation error";
        case SAMRENA_ERROR_PLATFORM_SPECIFIC:
            return "Platform specific error";
        default:
            return "Unknown error";
    }
}

static void samrena_set_error(SamrenaError error) { 
    last_error = error; 
}

// Stub implementations for Phase 1 (to be implemented in Phase 2)
Samrena* samrena_create(const SamrenaConfig* config) {
    samrena_set_error(SAMRENA_ERROR_UNSUPPORTED_STRATEGY);
    (void)config; // Avoid unused parameter warning
    return NULL;
}

void samrena_destroy(Samrena* arena) {
    (void)arena; // Avoid unused parameter warning
}

void* samrena_push(Samrena* arena, uint64_t size) {
    samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
    (void)arena; // Avoid unused parameter warning
    (void)size;  // Avoid unused parameter warning
    return NULL;
}

void* samrena_push_zero(Samrena* arena, uint64_t size) {
    samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
    (void)arena; // Avoid unused parameter warning
    (void)size;  // Avoid unused parameter warning
    return NULL;
}

uint64_t samrena_allocated(Samrena* arena) {
    (void)arena; // Avoid unused parameter warning
    return 0;
}

uint64_t samrena_capacity(Samrena* arena) {
    (void)arena; // Avoid unused parameter warning
    return 0;
}

// Temporary legacy API compatibility (will be removed in Phase 4)
Samrena* samrena_allocate(uint64_t page_count) {
    (void)page_count; // Avoid unused parameter warning
    samrena_set_error(SAMRENA_ERROR_UNSUPPORTED_STRATEGY);
    return NULL;
}

void samrena_deallocate(Samrena* samrena) {
    (void)samrena; // Avoid unused parameter warning
}

void* samrena_resize_array(Samrena* samrena, void* original_array, uint64_t original_size, uint64_t new_size) {
    (void)samrena; // Avoid unused parameter warning
    (void)original_array; // Avoid unused parameter warning
    (void)original_size; // Avoid unused parameter warning
    (void)new_size; // Avoid unused parameter warning
    samrena_set_error(SAMRENA_ERROR_UNSUPPORTED_OPERATION);
    return NULL;
}