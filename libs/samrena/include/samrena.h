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
        .enable_debug = false
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
        .enable_debug = false \
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
        .enable_debug = false \
    })

// Error handling functions
SamrenaError samrena_get_last_error(void);
const char *samrena_error_string(SamrenaError error);

// Hexagonal architecture API
Samrena* samrena_create(const SamrenaConfig* config);
void samrena_destroy(Samrena* arena);
void* samrena_push(Samrena* arena, uint64_t size);
void* samrena_push_zero(Samrena* arena, uint64_t size);
uint64_t samrena_allocated(Samrena* arena);
uint64_t samrena_capacity(Samrena* arena);

// Temporary legacy API compatibility (will be removed in Phase 4)
Samrena* samrena_allocate(uint64_t page_count);
void samrena_deallocate(Samrena* samrena);
void* samrena_resize_array(Samrena* samrena, void* original_array, uint64_t original_size, uint64_t new_size);

#endif
