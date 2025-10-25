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

// Error codes
typedef enum {
  SAMRENA_SUCCESS = 0,
  SAMRENA_ERROR_NULL_POINTER,
  SAMRENA_ERROR_INVALID_SIZE,
  SAMRENA_ERROR_OUT_OF_MEMORY,
  SAMRENA_ERROR_INVALID_PARAMETER,
  SAMRENA_ERROR_UNSUPPORTED_OPERATION
} SamrenaError;

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

// Samrena arena handle
typedef struct {
  SamrenaImpl *impl;
  void *context; // Virtual memory context
} Samrena;

// =============================================================================
// CONFIGURATION STRUCTURES
// =============================================================================

// Configuration structure for virtual memory arena
typedef struct {
  // Common parameters
  uint64_t initial_pages; // Initial allocation in pages
  uint64_t page_size;     // Page size (0 = default)

  // Virtual memory parameters
  uint64_t max_reserve; // Maximum virtual space (0 = default 64MB)
  uint64_t commit_size; // Commit granularity (0 = page_size)

  // Optional features
  bool enable_stats; // Track allocation statistics
  bool enable_debug; // Enable debug features

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
  uint64_t allocated;
  uint64_t capacity;
  uint64_t page_size;
  bool is_contiguous;
} SamrenaInfo;

// =============================================================================
// CONFIGURATION HELPERS
// =============================================================================

// Configuration helper functions
static inline SamrenaConfig samrena_default_config(void) {
  return (SamrenaConfig){.initial_pages = 1,
                         .page_size = 0, // Use system default
                         .max_reserve = 0, // Use default (64MB)
                         .commit_size = 0,
                         .enable_stats = false,
                         .enable_debug = false,
                         .log_callback = NULL,
                         .log_user_data = NULL};
}

// =============================================================================
// INTERNAL STRUCTURES
// =============================================================================

// Virtual memory context - internal implementation details
typedef struct {
  void *base_address;
  uint64_t reserved_size;
  uint64_t committed_size;
  uint64_t allocated_size;
  uint64_t commit_granularity;
  uint64_t page_size;
  bool enable_stats;
  bool enable_debug;
} VirtualContext;

// Implementation structure
struct SamrenaImpl {
  uint64_t page_size;
  SamrenaConfig config;

  // Statistics
  struct {
    uint64_t total_allocations;
    uint64_t failed_allocations;
    uint64_t peak_usage;
  } stats;
};

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
