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

#ifndef _WIN32
#define _GNU_SOURCE
#endif

#include "samrena.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

static SamrenaError last_error = SAMRENA_SUCCESS;

// =============================================================================
// Error Handling
// =============================================================================

SamrenaError samrena_get_last_error(void) { return last_error; }

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
    case SAMRENA_ERROR_INVALID_PARAMETER:
      return "Invalid parameter error";
    case SAMRENA_ERROR_UNSUPPORTED_OPERATION:
      return "Unsupported operation error";
    default:
      return "Unknown error";
  }
}

static void samrena_set_error(SamrenaError error) { last_error = error; }

// =============================================================================
// Virtual Memory Platform Abstraction
// =============================================================================

static uint64_t get_system_page_size(void) {
#ifdef _WIN32
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwPageSize;
#else
  return (uint64_t)sysconf(_SC_PAGESIZE);
#endif
}

static uint64_t get_allocation_granularity(void) {
#ifdef _WIN32
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwAllocationGranularity;
#else
  return get_system_page_size();
#endif
}

static void *virtual_reserve_memory(uint64_t size) {
#ifdef _WIN32
  return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
#else
  void *addr = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  return (addr == MAP_FAILED) ? NULL : addr;
#endif
}

static bool virtual_commit(void *address, uint64_t size) {
#ifdef _WIN32
  return VirtualAlloc(address, size, MEM_COMMIT, PAGE_READWRITE) != NULL;
#else
  return mprotect(address, size, PROT_READ | PROT_WRITE) == 0;
#endif
}

static void virtual_release(void *address, uint64_t size) {
#ifdef _WIN32
  VirtualFree(address, 0, MEM_RELEASE);
  (void)size; // Unused on Windows
#else
  munmap(address, size);
#endif
}

static void virtual_decommit_physical(void *address, uint64_t size) {
#ifdef _WIN32
  // On Windows, use VirtualAlloc with MEM_RESET to tell OS it can decommit pages
  // Pages remain committed but OS can reclaim physical memory
  VirtualAlloc(address, size, MEM_RESET, PAGE_READWRITE);
#elif defined(__linux__)
  // On Linux, MADV_DONTNEED tells kernel to free physical pages immediately
  // but keep virtual mapping - pages will be zero-filled on next access
  madvise(address, size, MADV_DONTNEED);
#elif defined(__APPLE__)
  // On macOS, MADV_FREE marks pages as reusable (lazier than DONTNEED)
  // Fall back to MADV_DONTNEED if MADV_FREE not available
#ifdef MADV_FREE
  madvise(address, size, MADV_FREE);
#else
  madvise(address, size, MADV_DONTNEED);
#endif
#else
  // Other Unix-like systems - try MADV_DONTNEED
  madvise(address, size, MADV_DONTNEED);
#endif
}

// =============================================================================
// Logging
// =============================================================================

static void log_message(const SamrenaConfig *config, const char *format, ...) {
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  if (config->log_callback) {
    config->log_callback(buffer, config->log_user_data);
  } else {
    fprintf(stderr, "samrena: %s\n", buffer);
  }
}

// =============================================================================
// Configuration Validation
// =============================================================================

static SamrenaError validate_config(const SamrenaConfig *config) {
  if (!config) {
    return SAMRENA_ERROR_NULL_POINTER;
  }

  if (config->initial_pages == 0) {
    return SAMRENA_ERROR_INVALID_PARAMETER;
  }

  if (config->page_size != 0 && config->page_size < 4096) {
    return SAMRENA_ERROR_INVALID_PARAMETER; // Minimum page size
  }

  return SAMRENA_SUCCESS;
}

// =============================================================================
// Arena Lifecycle
// =============================================================================

Samrena *samrena_create(const SamrenaConfig *config) {
  // Use defaults if no config provided
  SamrenaConfig cfg = config ? *config : samrena_default_config();

  // Validate configuration
  SamrenaError err = validate_config(&cfg);
  if (err != SAMRENA_SUCCESS) {
    if (cfg.log_callback) {
      log_message(&cfg, "Invalid configuration: error %d", err);
    }
    samrena_set_error(err);
    return NULL;
  }

  // Apply defaults for zero values
  if (cfg.page_size == 0) {
    cfg.page_size = 64 * 1024; // 64KB default page size
  }

  // Allocate arena structure
  Samrena *arena = calloc(1, sizeof(Samrena));
  if (!arena) {
    samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
    return NULL;
  }

  // Initialize arena
  arena->page_size = cfg.page_size;
  arena->config = cfg;

  // Initialize virtual context
  VirtualContext *ctx = &arena->vctx;

  ctx->page_size = get_system_page_size();
  ctx->commit_granularity = cfg.commit_size > 0 ? cfg.commit_size : ctx->page_size;
  ctx->enable_stats = cfg.enable_stats;
  ctx->enable_debug = cfg.enable_debug;

  // Default to 256MB if not specified
  ctx->reserved_size = cfg.max_reserve > 0 ? cfg.max_reserve : (256ULL * 1024 * 1024);

  // Align to allocation granularity
  uint64_t granularity = get_allocation_granularity();
  ctx->reserved_size = (ctx->reserved_size + granularity - 1) & ~(granularity - 1);

  // Reserve virtual address space
  ctx->base_address = virtual_reserve_memory(ctx->reserved_size);
  if (!ctx->base_address) {
    free(arena);
    samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
    return NULL;
  }

  ctx->committed_size = 0;
  ctx->allocated_size = 0;

  // Commit initial pages
  uint64_t initial_commit = cfg.initial_pages * ctx->page_size;
  if (initial_commit > 0) {
    if (!virtual_commit(ctx->base_address, initial_commit)) {
      virtual_release(ctx->base_address, ctx->reserved_size);
      free(arena);
      samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
      return NULL;
    }
    ctx->committed_size = initial_commit;
  }
  samrena_set_error(SAMRENA_SUCCESS);
  return arena;
}

void samrena_destroy(Samrena *arena) {
  if (!arena)
    return;

  VirtualContext *ctx = &arena->vctx;
  if (ctx->base_address) {
    virtual_release(ctx->base_address, ctx->reserved_size);
  }

  free(arena);
}

// =============================================================================
// Core Allocation Functions
// =============================================================================

void *samrena_push(Samrena *arena, uint64_t size) {
  if (!arena) {
    samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
    return NULL;
  }

  if (size == 0) {
    samrena_set_error(SAMRENA_ERROR_INVALID_SIZE);
    return NULL;
  }

  VirtualContext *ctx = &arena->vctx;

  // Align size to 8-byte boundary
  size = (size + 7) & ~7;

  uint64_t new_allocated = ctx->allocated_size + size;

  // Check if we exceed reserved space
  if (new_allocated > ctx->reserved_size) {
    samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
    return NULL;
  }

  // Check if we need to commit more memory
  if (new_allocated > ctx->committed_size) {
    uint64_t needed_commit = new_allocated - ctx->committed_size;
    uint64_t commit_size =
        ((needed_commit + ctx->commit_granularity - 1) / ctx->commit_granularity) *
        ctx->commit_granularity;

    uint64_t new_committed = ctx->committed_size + commit_size;
    if (new_committed > ctx->reserved_size) {
      new_committed = ctx->reserved_size;
      commit_size = new_committed - ctx->committed_size;
    }

    if (commit_size > 0) {
      void *commit_addr = (uint8_t *)ctx->base_address + ctx->committed_size;
      if (!virtual_commit(commit_addr, commit_size)) {
        samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
        return NULL;
      }
      ctx->committed_size = new_committed;
    }
  }

  void *result = (uint8_t *)ctx->base_address + ctx->allocated_size;
  ctx->allocated_size = new_allocated;

  samrena_set_error(SAMRENA_SUCCESS);
  return result;
}

void *samrena_push_zero(Samrena *arena, uint64_t size) {
  void *result = samrena_push(arena, size);
  if (result) {
    memset(result, 0, size);
  }
  return result;
}

void *samrena_push_aligned(Samrena *arena, uint64_t size, uint64_t alignment) {
  if (!arena) {
    samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
    return NULL;
  }

  if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
    // Alignment must be a power of 2
    samrena_set_error(SAMRENA_ERROR_INVALID_PARAMETER);
    return NULL;
  }

  // Allocate a byte to get current position, then calculate padding
  void *temp_ptr = samrena_push(arena, 1);
  if (!temp_ptr) {
    return NULL;
  }

  uintptr_t current_addr = (uintptr_t)temp_ptr;
  uintptr_t aligned_addr = (current_addr + alignment - 1) & ~(alignment - 1);
  uint64_t padding = aligned_addr - current_addr;

  // Allocate additional padding if needed (we already allocated 1 byte)
  if (padding > 1) {
    void *padding_ptr = samrena_push(arena, padding - 1);
    if (!padding_ptr) {
      return NULL;
    }
  }

  // Now allocate the actual data (the first byte of the aligned block is our temp byte)
  if (size > 1) {
    void *data_ptr = samrena_push(arena, size - 1);
    if (!data_ptr) {
      return NULL;
    }
  }

  // Return the aligned address
  return (void *)aligned_addr;
}

// =============================================================================
// Query Functions
// =============================================================================

uint64_t samrena_allocated(Samrena *arena) {
  if (!arena) {
    samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
    return 0;
  }

  VirtualContext *ctx = &arena->vctx;
  return ctx->allocated_size;
}

uint64_t samrena_capacity(Samrena *arena) {
  if (!arena) {
    samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
    return 0;
  }

  VirtualContext *ctx = &arena->vctx;
  return ctx->committed_size;
}

void samrena_get_info(Samrena *arena, SamrenaInfo *info) {
  if (!arena || !info)
    return;

  info->allocated = samrena_allocated(arena);
  info->capacity = samrena_capacity(arena);
  info->page_size = arena->page_size;
  info->is_contiguous = true;
}

// =============================================================================
// Factory Functions
// =============================================================================

Samrena *samrena_create_default(void) {
  return samrena_create(NULL); // Use all defaults
}

Samrena *samrena_create_global(void) {
  SamrenaConfig config = samrena_default_config();
  config.max_reserve = 4ULL * 1024 * 1024 * 1024 * 1024; // 4TB
  return samrena_create(&config);
}

Samrena *samrena_create_session(void) {
  SamrenaConfig config = samrena_default_config();
  config.max_reserve = 256ULL * 1024 * 1024 * 1024; // 256GB
  return samrena_create(&config);
}

// =============================================================================
// Capability Query API
// =============================================================================

static const SamrenaCapabilities virtual_capabilities = {
    .flags = SAMRENA_CAP_CONTIGUOUS_MEMORY | SAMRENA_CAP_ZERO_COPY_GROWTH | SAMRENA_CAP_RESET |
             SAMRENA_CAP_RESERVE,
    .max_allocation_size = 0, // Set dynamically based on reserved size
    .alignment_guarantee = 16};

SamrenaCapabilities samrena_get_capabilities(Samrena *arena) {
  if (!arena) {
    return (SamrenaCapabilities){0};
  }

  VirtualContext *ctx = &arena->vctx;
  SamrenaCapabilities caps = virtual_capabilities;

  // Set dynamic values based on current state
  caps.max_allocation_size = ctx->reserved_size - ctx->allocated_size;

  return caps;
}

bool samrena_has_capability(Samrena *arena, SamrenaCapabilityFlags cap) {
  SamrenaCapabilities caps = samrena_get_capabilities(arena);
  return (caps.flags & cap) != 0;
}

// =============================================================================
// Memory Management API
// =============================================================================

SamrenaError samrena_reserve(Samrena *arena, uint64_t min_capacity) {
  if (!arena) {
    return SAMRENA_ERROR_INVALID_PARAMETER;
  }

  VirtualContext *ctx = &arena->vctx;

  if (min_capacity > ctx->reserved_size) {
    return SAMRENA_ERROR_INVALID_PARAMETER;
  }

  if (ctx->committed_size >= min_capacity) {
    return SAMRENA_SUCCESS;
  }

  uint64_t needed = min_capacity - ctx->committed_size;
  uint64_t commit_size =
      ((needed + ctx->commit_granularity - 1) / ctx->commit_granularity) * ctx->commit_granularity;

  uint64_t new_committed = ctx->committed_size + commit_size;
  if (new_committed > ctx->reserved_size) {
    new_committed = ctx->reserved_size;
    commit_size = new_committed - ctx->committed_size;
  }

  if (commit_size > 0) {
    void *commit_addr = (uint8_t *)ctx->base_address + ctx->committed_size;
    if (!virtual_commit(commit_addr, commit_size)) {
      return SAMRENA_ERROR_OUT_OF_MEMORY;
    }
    ctx->committed_size = new_committed;
  }

  return SAMRENA_SUCCESS;
}

SamrenaError samrena_reserve_with_growth(Samrena *arena, uint64_t immediate_size,
                                         uint64_t expected_total) {
  if (!arena) {
    return SAMRENA_ERROR_INVALID_PARAMETER;
  }

  // Reserve immediate size plus some headroom
  uint64_t reserve_size = immediate_size * 2;
  if (reserve_size < expected_total / 4) {
    reserve_size = expected_total / 4; // Reserve 25% of expected
  }

  return samrena_reserve(arena, reserve_size);
}

bool samrena_can_allocate(Samrena *arena, uint64_t size) {
  if (!arena)
    return false;

  VirtualContext *ctx = &arena->vctx;
  uint64_t used = ctx->allocated_size;
  uint64_t capacity = ctx->reserved_size;
  return (used + size) <= capacity;
}

bool samrena_reset_if_supported(Samrena *arena) {
  if (!arena) {
    return false;
  }

  VirtualContext *ctx = &arena->vctx;

  // Tell OS it can reclaim physical pages but keep virtual mapping
  // This releases physical memory while keeping the address space reserved
  if (ctx->committed_size > 0) {
    virtual_decommit_physical(ctx->base_address, ctx->committed_size);
  }

  ctx->allocated_size = 0;
  return true;
}
