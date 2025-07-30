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

#ifndef SAMRENA_INTERNAL_H
#define SAMRENA_INTERNAL_H

#include "samrena.h"
#include <stdio.h>

// Operations structure - the core of the hexagonal architecture
typedef struct {
    const char* name;  // Adapter name for debugging
    
    // Lifecycle operations
    SamrenaError (*create)(void** context, const void* config);
    void (*destroy)(void* context);
    
    // Core memory operations
    void* (*push)(void* context, uint64_t size);
    void* (*push_zero)(void* context, uint64_t size);
    
    // Query operations
    uint64_t (*allocated)(void* context);
    uint64_t (*capacity)(void* context);
    
    // Optional operations (can be NULL)
    SamrenaError (*reserve)(void* context, uint64_t min_capacity);
    void (*reset)(void* context);
    
    // Capability query
    const SamrenaCapabilities* (*get_capabilities)(void* context);
    
    // Optional advanced operations
    SamrenaError (*save_point)(void* context, void** savepoint);
    SamrenaError (*restore_point)(void* context, void* savepoint);
    
    // Performance hints
    void (*prefetch)(void* context, uint64_t expected_size);
    
    // Debug operations (optional)
    void (*dump_stats)(void* context, FILE* out);
} SamrenaOps;

// Implementation structure - private to samrena
struct SamrenaImpl {
    const SamrenaOps* ops;
    SamrenaStrategy strategy;
    uint64_t page_size;
    SamrenaConfig config;
    
    // Growth hint for future allocations
    uint64_t growth_hint;
    
    // Statistics (common to all adapters)
    struct {
        uint64_t total_allocations;
        uint64_t failed_allocations;
        uint64_t peak_usage;
    } stats;
    
    // Debug info
    char adapter_name[32];
};

// Operation helpers for SamrenaArena
#define SAMRENA_HAS_OP(arena, op) \
    ((arena)->impl->ops->op != NULL)

#define SAMRENA_CALL_OP(arena, op, ...) \
    (SAMRENA_HAS_OP(arena, op) ? \
     (arena)->impl->ops->op((arena)->context, ##__VA_ARGS__) : \
     SAMRENA_ERROR_UNSUPPORTED_OPERATION)

// Safe operation call with result type checking
#define SAMRENA_CALL_OP_VOID(arena, op, ...) \
    do { \
        if (SAMRENA_HAS_OP(arena, op)) { \
            (arena)->impl->ops->op((arena)->context, ##__VA_ARGS__); \
        } \
    } while(0)

#define SAMRENA_CALL_OP_PTR(arena, op, ...) \
    (SAMRENA_HAS_OP(arena, op) ? \
     (arena)->impl->ops->op((arena)->context, ##__VA_ARGS__) : \
     NULL)

#define SAMRENA_CALL_OP_UINT64(arena, op, ...) \
    (SAMRENA_HAS_OP(arena, op) ? \
     (arena)->impl->ops->op((arena)->context, ##__VA_ARGS__) : \
     0)

// Forward declarations for hexagonal API (to be implemented in Phase 4)
Samrena* samrena_create(const SamrenaConfig* config);
void samrena_destroy(Samrena* arena);
void* samrena_push_new(Samrena* arena, uint64_t size);
void* samrena_push_zero_new(Samrena* arena, uint64_t size);
uint64_t samrena_allocated_new(Samrena* arena);
uint64_t samrena_capacity_new(Samrena* arena);

#endif // SAMRENA_INTERNAL_H