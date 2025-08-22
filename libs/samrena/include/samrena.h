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

// =============================================================================
// STANDARD INCLUDES
// =============================================================================

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// =============================================================================
// CONSTANTS AND DEFINES
// =============================================================================

#define NEAT_INITIAL_PAGE_SIZE 1024

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================

typedef struct SamrenaImpl SamrenaImpl;

// =============================================================================
// CORE ENUMERATIONS
// =============================================================================

// Strategy enumeration for adapter selection
typedef enum {
  SAMRENA_STRATEGY_DEFAULT = 0,
  SAMRENA_STRATEGY_CHAINED,
  SAMRENA_STRATEGY_VIRTUAL
} SamrenaStrategy;

// Error codes
typedef enum {
  SAMRENA_SUCCESS = 0,
  SAMRENA_ERROR_NULL_POINTER,
  SAMRENA_ERROR_INVALID_SIZE,
  SAMRENA_ERROR_OUT_OF_MEMORY,
  SAMRENA_ERROR_INVALID_PARAMETER,
  SAMRENA_ERROR_UNSUPPORTED_STRATEGY,
  SAMRENA_ERROR_UNSUPPORTED_OPERATION
} SamrenaError;

// Fallback behavior enumeration
typedef enum {
  SAMRENA_FALLBACK_AUTO = 0, // Automatically choose best alternative
  SAMRENA_FALLBACK_WARN,     // Fallback with warning
  SAMRENA_FALLBACK_STRICT    // Fail if exact strategy unavailable
} SamrenaFallbackMode;

// =============================================================================
// GROWTH POLICY ENUMERATIONS
// =============================================================================

// Growth policy types
typedef enum {
  SAMRENA_GROWTH_LINEAR,     // Add fixed amount each time
  SAMRENA_GROWTH_EXPONENTIAL // Double size each time
} SamrenaGrowthPolicy;

// =============================================================================
// CAPABILITY ENUMERATIONS
// =============================================================================

// Capability flags for adapter features
typedef enum {
  SAMRENA_CAP_CONTIGUOUS_MEMORY = 1 << 0, // All allocations in one block
  SAMRENA_CAP_ZERO_COPY_GROWTH = 1 << 1,  // Can grow without moving data
  SAMRENA_CAP_RESET = 1 << 2,             // Supports reset operation
  SAMRENA_CAP_RESERVE = 1 << 3            // Supports reserve operation
} SamrenaCapabilityFlags;

// =============================================================================
// CORE STRUCTURES
// =============================================================================

// Hexagonal architecture Samrena handle
typedef struct {
  SamrenaImpl *impl;
  void *context; // Implementation-specific data
} Samrena;

// =============================================================================
// GROWTH POLICY STRUCTURES
// =============================================================================

// Growth policy configuration
typedef struct {
  SamrenaGrowthPolicy policy;
  union {
    struct {
      uint64_t increment; // For linear growth
    } linear;
    struct {
      double factor; // For exponential (default 2.0)
    } exponential;
  } params;
} SamrenaGrowthConfig;

// =============================================================================
// CONFIGURATION STRUCTURES
// =============================================================================

// Configuration structure for all adapter types
typedef struct {
  // Strategy selection
  SamrenaStrategy strategy;

  // Common parameters
  uint64_t initial_pages; // Initial allocation in pages
  uint64_t page_size;     // Page size (0 = default)

  // Chained adapter specific
  uint64_t growth_pages; // Pages to add on expansion

  // Virtual adapter specific
  uint64_t max_reserve; // Maximum virtual space (0 = default)
  uint64_t commit_size; // Commit granularity (0 = page_size)

  // Optional features
  bool enable_stats; // Track allocation statistics
  bool enable_debug; // Enable debug features

  // Fallback behavior
  SamrenaFallbackMode fallback_mode;

  // Growth policy
  SamrenaGrowthConfig growth;

  // Logging callback
  void (*log_callback)(const char *message, void *user_data);
  void *log_user_data;
} SamrenaConfig;

// =============================================================================
// INFORMATION STRUCTURES
// =============================================================================

// Capability information structure
typedef struct {
  uint32_t flags;               // Capability flags
  uint64_t max_allocation_size; // Largest single allocation
  uint64_t alignment_guarantee; // Guaranteed alignment
} SamrenaCapabilities;

// Arena information
typedef struct {
  const char *adapter_name;
  SamrenaStrategy strategy;
  uint64_t allocated;
  uint64_t capacity;
  uint64_t page_size;
  bool can_grow;
  bool is_contiguous;
} SamrenaInfo;

// =============================================================================
// CONFIGURATION HELPERS
// =============================================================================

// Configuration helper functions
static inline SamrenaConfig samrena_default_config(void) {
  SamrenaGrowthConfig default_growth = {.policy = SAMRENA_GROWTH_EXPONENTIAL,
                                        .params.exponential = {.factor = 1.5}};

  return (SamrenaConfig){.strategy = SAMRENA_STRATEGY_DEFAULT,
                         .initial_pages = 1,
                         .page_size = 0, // Use system default
                         .growth_pages = 1,
                         .max_reserve = 0, // Use adapter default
                         .commit_size = 0,
                         .enable_stats = false,
                         .enable_debug = false,
                         .fallback_mode = SAMRENA_FALLBACK_WARN,
                         .growth = default_growth,
                         .log_callback = NULL,
                         .log_user_data = NULL};
}

// Configuration builder functions
SamrenaConfig samrena_config_chained(uint64_t pages);
SamrenaConfig samrena_config_virtual(uint64_t reserve_mb);

// Preset growth configurations
extern const SamrenaGrowthConfig SAMRENA_GROWTH_CONSERVATIVE;
extern const SamrenaGrowthConfig SAMRENA_GROWTH_BALANCED;

// =============================================================================
// CORE API - Arena Management
// =============================================================================

// Primary arena lifecycle functions
Samrena *samrena_create(const SamrenaConfig *config);
void samrena_destroy(Samrena *arena);

// Core allocation functions
void *samrena_push(Samrena *arena, uint64_t size);
void *samrena_push_zero(Samrena *arena, uint64_t size);
void *samrena_push_aligned(Samrena *arena, uint64_t size, uint64_t alignment);

// Arena information functions
uint64_t samrena_allocated(Samrena *arena);
uint64_t samrena_capacity(Samrena *arena);
void samrena_get_info(Samrena *arena, SamrenaInfo *info);

// =============================================================================
// FACTORY API - Convenient Arena Creation
// =============================================================================

// Factory functions for common use cases
Samrena *samrena_create_default(void);

// =============================================================================
// CAPABILITY API - Feature Detection
// =============================================================================

SamrenaCapabilities samrena_get_capabilities(Samrena *arena);
bool samrena_has_capability(Samrena *arena, SamrenaCapabilityFlags cap);
SamrenaCapabilities samrena_strategy_capabilities(SamrenaStrategy strategy);

// =============================================================================
// STRATEGY API - Strategy Management
// =============================================================================

bool samrena_strategy_available(SamrenaStrategy strategy);
int samrena_available_strategies(SamrenaStrategy *strategies, int max_count);
const char *samrena_strategy_name(SamrenaStrategy strategy);

// =============================================================================
// MEMORY MANAGEMENT API - Advanced Operations
// =============================================================================

SamrenaError samrena_reserve(Samrena *arena, uint64_t size);
SamrenaError samrena_reserve_with_growth(Samrena *arena, uint64_t immediate_size,
                                         uint64_t expected_total);
bool samrena_can_allocate(Samrena *arena, uint64_t size);
bool samrena_reset_if_supported(Samrena *arena);

// =============================================================================
// ERROR HANDLING API
// =============================================================================

SamrenaError samrena_get_last_error(void);
const char *samrena_error_string(SamrenaError error);

// =============================================================================
// TYPE-SAFE MACROS - Convenience Macros for Common Operations
// =============================================================================

#define SAMRENA_PUSH_TYPE(arena, type) ((type *)samrena_push((arena), sizeof(type)))

#define SAMRENA_PUSH_ARRAY(arena, type, count)                                                     \
  ((type *)samrena_push((arena), sizeof(type) * (count)))

#define SAMRENA_PUSH_TYPE_ZERO(arena, type) ((type *)samrena_push_zero((arena), sizeof(type)))

#define SAMRENA_PUSH_ARRAY_ZERO(arena, type, count)                                                \
  ((type *)samrena_push_zero((arena), sizeof(type) * (count)))

#define SAMRENA_PUSH_ALIGNED_TYPE(arena, type, alignment)                                          \
  ((type *)samrena_push_aligned((arena), sizeof(type), (alignment)))

#endif
