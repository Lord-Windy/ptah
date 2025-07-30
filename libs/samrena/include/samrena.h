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

#ifndef SAMRENA_H
#define SAMRENA_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define NEAT_INITIAL_PAGE_SIZE 1024

// Forward declaration of implementation for hexagonal architecture
typedef struct SamrenaImpl SamrenaImpl;

// Strategy enumeration for adapter selection
typedef enum {
    SAMRENA_STRATEGY_DEFAULT = 0,
    SAMRENA_STRATEGY_CHAINED,
    SAMRENA_STRATEGY_VIRTUAL
} SamrenaStrategy;

// Enhanced error codes for hexagonal architecture
typedef enum {
  // Legacy error codes (preserved for compatibility)
  SAMRENA_SUCCESS = 0,
  SAMRENA_ERROR_NULL_POINTER = 1,
  SAMRENA_ERROR_INVALID_SIZE = 2,
  SAMRENA_ERROR_OUT_OF_MEMORY = 3,
  SAMRENA_ERROR_OVERFLOW = 4,
  
  // New error codes for hexagonal architecture
  SAMRENA_ERROR_INVALID_PARAMETER = 5,
  SAMRENA_ERROR_UNSUPPORTED_STRATEGY = 6,
  SAMRENA_ERROR_UNSUPPORTED_OPERATION = 7,
  SAMRENA_ERROR_PLATFORM_SPECIFIC = 8
} SamrenaError;

// Hexagonal architecture Samrena handle
typedef struct {
    SamrenaImpl* impl;
    void* context;  // Implementation-specific data
} Samrena;

// Fallback behavior enumeration
typedef enum {
    SAMRENA_FALLBACK_AUTO = 0,     // Automatically choose best alternative
    SAMRENA_FALLBACK_WARN,         // Fallback with warning
    SAMRENA_FALLBACK_STRICT        // Fail if exact strategy unavailable
} SamrenaFallbackMode;

// Configuration structure for all adapter types
typedef struct {
    // Strategy selection
    SamrenaStrategy strategy;
    
    // Common parameters
    uint64_t initial_pages;      // Initial allocation in pages
    uint64_t page_size;          // Page size (0 = default)
    
    // Chained adapter specific
    uint64_t growth_pages;       // Pages to add on expansion
    
    // Virtual adapter specific
    uint64_t max_reserve;        // Maximum virtual space (0 = default)
    uint64_t commit_size;        // Commit granularity (0 = page_size)
    
    // Optional features
    bool enable_stats;           // Track allocation statistics
    bool enable_debug;           // Enable debug features
    
    // Fallback behavior
    SamrenaFallbackMode fallback_mode;
    
    // Logging callback
    void (*log_callback)(const char* message, void* user_data);
    void* log_user_data;
} SamrenaConfig;

typedef struct {
  uint64_t size;
  uint64_t element_size;
  uint64_t capacity;
  void *data;
} SamrenaVector;

// Configuration helper functions
static inline SamrenaConfig samrena_default_config(void) {
    return (SamrenaConfig){
        .strategy = SAMRENA_STRATEGY_DEFAULT,
        .initial_pages = 1,
        .page_size = 0,  // Use system default
        .growth_pages = 1,
        .max_reserve = 0,  // Use adapter default
        .commit_size = 0,
        .enable_stats = false,
        .enable_debug = false,
        .fallback_mode = SAMRENA_FALLBACK_WARN,
        .log_callback = NULL,
        .log_user_data = NULL
    };
}

// Configuration builder helpers
#define SAMRENA_CONFIG_CHAINED(pages) \
    ((SamrenaConfig){ \
        .strategy = SAMRENA_STRATEGY_CHAINED, \
        .initial_pages = pages, \
        .page_size = 0, \
        .growth_pages = 1, \
        .max_reserve = 0, \
        .commit_size = 0, \
        .enable_stats = false, \
        .enable_debug = false, \
        .fallback_mode = SAMRENA_FALLBACK_WARN, \
        .log_callback = NULL, \
        .log_user_data = NULL \
    })

#define SAMRENA_CONFIG_VIRTUAL(reserve_mb) \
    ((SamrenaConfig){ \
        .strategy = SAMRENA_STRATEGY_VIRTUAL, \
        .initial_pages = 1, \
        .page_size = 0, \
        .growth_pages = 1, \
        .max_reserve = (reserve_mb) * 1024 * 1024, \
        .commit_size = 0, \
        .enable_stats = false, \
        .enable_debug = false, \
        .fallback_mode = SAMRENA_FALLBACK_WARN, \
        .log_callback = NULL, \
        .log_user_data = NULL \
    })

// Error handling functions
SamrenaError samrena_get_last_error(void);
const char *samrena_error_string(SamrenaError error);

// Allocation hints for automatic strategy selection
typedef struct {
    // Expected usage pattern
    uint64_t expected_total_size;    // Total memory to be allocated
    uint64_t expected_max_alloc;     // Largest single allocation
    uint32_t expected_alloc_count;   // Number of allocations
    
    // Performance requirements
    bool require_contiguous;         // Need contiguous memory
    bool require_zero_copy_growth;   // Cannot tolerate reallocation
    bool frequent_reset;             // Will reset/clear often
    
    // System constraints
    uint64_t max_memory_limit;       // Hard memory limit
    bool low_memory_mode;            // Optimize for memory usage
} SamrenaAllocationHints;

// Hexagonal architecture API
Samrena* samrena_create(const SamrenaConfig* config);
void samrena_destroy(Samrena* arena);
void* samrena_push(Samrena* arena, uint64_t size);
void* samrena_push_zero(Samrena* arena, uint64_t size);
uint64_t samrena_allocated(Samrena* arena);
uint64_t samrena_capacity(Samrena* arena);

// Factory and strategy selection API
Samrena* samrena_create_with_hints(const SamrenaAllocationHints* hints);
Samrena* samrena_create_default(void);
Samrena* samrena_create_for_size(uint64_t expected_size);
Samrena* samrena_create_high_performance(uint64_t initial_mb);
Samrena* samrena_create_low_memory(void);
Samrena* samrena_create_temp(void);

// Vector API
SamrenaVector* samrena_vector_init(Samrena* arena, uint64_t element_size, uint64_t initial_capacity);
void* samrena_vector_push(Samrena* arena, SamrenaVector* vec, const void* element);
void* samrena_vector_pop(SamrenaVector* vec);
void* samrena_vector_resize(Samrena* arena, SamrenaVector* vec, uint64_t new_capacity);

// Performance hints structure
typedef struct {
    bool contiguous_memory;    // All allocations in single block
    bool zero_copy_growth;     // Can grow without copying
    bool constant_time_alloc;  // O(1) allocation
    uint64_t max_single_alloc; // Largest supported allocation
} SamrenaPerformanceHints;

// Capability query API
bool samrena_strategy_available(SamrenaStrategy strategy);
int samrena_available_strategies(SamrenaStrategy* strategies, int max_count);
const char* samrena_strategy_name(SamrenaStrategy strategy);
SamrenaPerformanceHints samrena_get_performance_hints(Samrena* arena);

// Convenience functions and utilities
bool samrena_can_allocate(Samrena* arena, uint64_t size);
void* samrena_push_aligned(Samrena* arena, uint64_t size, uint64_t alignment);
bool samrena_reset_if_supported(Samrena* arena);

// Arena information
typedef struct {
    const char* adapter_name;
    SamrenaStrategy strategy;
    uint64_t allocated;
    uint64_t capacity;
    uint64_t page_size;
    bool can_grow;
    bool is_contiguous;
} SamrenaInfo;

void samrena_get_info(Samrena* arena, SamrenaInfo* info);

// Type-safe allocation macros
#define SAMRENA_PUSH_TYPE(arena, type) \
    ((type*)samrena_push((arena), sizeof(type)))

#define SAMRENA_PUSH_ARRAY(arena, type, count) \
    ((type*)samrena_push((arena), sizeof(type) * (count)))

#define SAMRENA_PUSH_TYPE_ZERO(arena, type) \
    ((type*)samrena_push_zero((arena), sizeof(type)))

#define SAMRENA_PUSH_ARRAY_ZERO(arena, type, count) \
    ((type*)samrena_push_zero((arena), sizeof(type) * (count)))

#define SAMRENA_PUSH_ALIGNED_TYPE(arena, type, alignment) \
    ((type*)samrena_push_aligned((arena), sizeof(type), (alignment)))

// Backward compatibility functions (maintained from legacy API)
Samrena* samrena_allocate(uint64_t page_count);
void samrena_deallocate(Samrena* samrena);
void* samrena_resize_array(Samrena* samrena, void* original_array, uint64_t original_size, uint64_t new_size);

#endif
