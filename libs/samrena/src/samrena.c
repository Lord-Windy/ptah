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
#include "adapters/chained_adapter.h"
#include "adapters/virtual_adapter.h"
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

static const SamrenaOps* get_adapter_ops(SamrenaStrategy strategy) {
    switch (strategy) {
        case SAMRENA_STRATEGY_DEFAULT:
        case SAMRENA_STRATEGY_CHAINED:
            return &chained_adapter_ops;
        case SAMRENA_STRATEGY_VIRTUAL:
            return &virtual_adapter_ops;
        default:
            return NULL;
    }
}

Samrena* samrena_create(const SamrenaConfig* config) {
    if (!config) {
        samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
        return NULL;
    }
    
    const SamrenaOps* ops = get_adapter_ops(config->strategy);
    if (!ops) {
        samrena_set_error(SAMRENA_ERROR_UNSUPPORTED_STRATEGY);
        return NULL;
    }
    
    // Allocate the arena structure
    Samrena* arena = malloc(sizeof(Samrena));
    if (!arena) {
        samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    // Allocate the implementation structure
    SamrenaImpl* impl = malloc(sizeof(SamrenaImpl));
    if (!impl) {
        free(arena);
        samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    impl->ops = ops;
    impl->config = *config;
    arena->impl = impl;
    arena->context = NULL;
    
    // Create the adapter context
    SamrenaError error = ops->create(&arena->context, config);
    if (error != SAMRENA_SUCCESS) {
        free(impl);
        free(arena);
        samrena_set_error(error);
        return NULL;
    }
    
    samrena_set_error(SAMRENA_SUCCESS);
    return arena;
}

void samrena_destroy(Samrena* arena) {
    if (!arena) return;
    
    if (arena->impl && arena->impl->ops && arena->impl->ops->destroy) {
        arena->impl->ops->destroy(arena->context);
    }
    
    free(arena->impl);
    free(arena);
}

void* samrena_push(Samrena* arena, uint64_t size) {
    if (!arena) {
        samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
        return NULL;
    }
    
    void* result = SAMRENA_CALL_OP_PTR(arena, push, size);
    if (!result) {
        samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
    } else {
        samrena_set_error(SAMRENA_SUCCESS);
    }
    
    return result;
}

void* samrena_push_zero(Samrena* arena, uint64_t size) {
    if (!arena) {
        samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
        return NULL;
    }
    
    void* result = SAMRENA_CALL_OP_PTR(arena, push_zero, size);
    if (!result) {
        samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
    } else {
        samrena_set_error(SAMRENA_SUCCESS);
    }
    
    return result;
}

uint64_t samrena_allocated(Samrena* arena) {
    if (!arena) {
        samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
        return 0;
    }
    
    return SAMRENA_CALL_OP_UINT64(arena, allocated);
}

uint64_t samrena_capacity(Samrena* arena) {
    if (!arena) {
        samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
        return 0;
    }
    
    return SAMRENA_CALL_OP_UINT64(arena, capacity);
}

// Temporary legacy API compatibility (will be removed in Phase 4)
Samrena* samrena_allocate(uint64_t page_count) {
    if (page_count == 0) {
        samrena_set_error(SAMRENA_ERROR_INVALID_SIZE);
        return NULL;
    }
    
    SamrenaConfig config = samrena_default_config();
    config.initial_pages = page_count;
    return samrena_create(&config);
}

void samrena_deallocate(Samrena* samrena) {
    samrena_destroy(samrena);
}

void* samrena_resize_array(Samrena* samrena, void* original_array, uint64_t original_size, uint64_t new_size) {
    if (!samrena) {
        samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
        return NULL;
    }
    
    if (new_size == 0) {
        samrena_set_error(SAMRENA_SUCCESS);
        // Return a valid pointer for zero-size allocation (common pattern)
        return samrena_push(samrena, 1);
    }
    
    void* new_array = samrena_push(samrena, new_size);
    if (!new_array) {
        return NULL;
    }
    
    if (original_array && original_size > 0) {
        uint64_t copy_size = original_size < new_size ? original_size : new_size;
        memcpy(new_array, original_array, copy_size);
    }
    
    return new_array;
}