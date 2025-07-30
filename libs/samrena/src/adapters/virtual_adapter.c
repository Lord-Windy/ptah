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

#include "virtual_adapter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

typedef struct {
    void* base_address;
    uint64_t reserved_size;
    uint64_t committed_size;
    uint64_t allocated_size;
    uint64_t commit_granularity;
    uint64_t page_size;
    bool enable_stats;
    bool enable_debug;
} VirtualContext;

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

static void* virtual_reserve_memory(uint64_t size) {
#ifdef _WIN32
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
#else
    void* addr = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (addr == MAP_FAILED) ? NULL : addr;
#endif
}

static bool virtual_commit(void* address, uint64_t size) {
#ifdef _WIN32
    return VirtualAlloc(address, size, MEM_COMMIT, PAGE_READWRITE) != NULL;
#else
    return mprotect(address, size, PROT_READ | PROT_WRITE) == 0;
#endif
}

static void virtual_release(void* address, uint64_t size) {
#ifdef _WIN32
    VirtualFree(address, 0, MEM_RELEASE);
    (void)size; // Unused on Windows
#else
    munmap(address, size);
#endif
}

static SamrenaError virtual_create(void** context, const void* config) {
    if (!context || !config) {
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    const SamrenaConfig* cfg = (const SamrenaConfig*)config;
    
    VirtualContext* ctx = malloc(sizeof(VirtualContext));
    if (!ctx) {
        return SAMRENA_ERROR_OUT_OF_MEMORY;
    }
    
    ctx->page_size = get_system_page_size();
    ctx->commit_granularity = cfg->commit_size > 0 ? cfg->commit_size : ctx->page_size;
    ctx->enable_stats = cfg->enable_stats;
    ctx->enable_debug = cfg->enable_debug;
    
    // Default to 64MB if not specified
    ctx->reserved_size = cfg->max_reserve > 0 ? cfg->max_reserve : (64 * 1024 * 1024);
    
    // Align to allocation granularity
    uint64_t granularity = get_allocation_granularity();
    ctx->reserved_size = (ctx->reserved_size + granularity - 1) & ~(granularity - 1);
    
    // Reserve virtual address space
    ctx->base_address = virtual_reserve_memory(ctx->reserved_size);
    if (!ctx->base_address) {
        free(ctx);
        return SAMRENA_ERROR_OUT_OF_MEMORY;
    }
    
    ctx->committed_size = 0;
    ctx->allocated_size = 0;
    
    // Commit initial pages
    uint64_t initial_commit = cfg->initial_pages * ctx->page_size;
    if (initial_commit > 0) {
        if (!virtual_commit(ctx->base_address, initial_commit)) {
            virtual_release(ctx->base_address, ctx->reserved_size);
            free(ctx);
            return SAMRENA_ERROR_OUT_OF_MEMORY;
        }
        ctx->committed_size = initial_commit;
    }
    
    *context = ctx;
    return SAMRENA_SUCCESS;
}

static void virtual_destroy(void* context) {
    if (!context) return;
    
    VirtualContext* ctx = (VirtualContext*)context;
    if (ctx->base_address) {
        virtual_release(ctx->base_address, ctx->reserved_size);
    }
    free(ctx);
}

static void* virtual_push(void* context, uint64_t size) {
    if (!context || size == 0) {
        return NULL;
    }
    
    VirtualContext* ctx = (VirtualContext*)context;
    
    // Align size to 8-byte boundary
    size = (size + 7) & ~7;
    
    uint64_t new_allocated = ctx->allocated_size + size;
    
    // Check if we exceed reserved space
    if (new_allocated > ctx->reserved_size) {
        return NULL;
    }
    
    // Check if we need to commit more memory
    if (new_allocated > ctx->committed_size) {
        uint64_t needed_commit = new_allocated - ctx->committed_size;
        uint64_t commit_size = ((needed_commit + ctx->commit_granularity - 1) / 
                               ctx->commit_granularity) * ctx->commit_granularity;
        
        uint64_t new_committed = ctx->committed_size + commit_size;
        if (new_committed > ctx->reserved_size) {
            new_committed = ctx->reserved_size;
            commit_size = new_committed - ctx->committed_size;
        }
        
        if (commit_size > 0) {
            void* commit_addr = (uint8_t*)ctx->base_address + ctx->committed_size;
            if (!virtual_commit(commit_addr, commit_size)) {
                return NULL;
            }
            ctx->committed_size = new_committed;
        }
    }
    
    void* result = (uint8_t*)ctx->base_address + ctx->allocated_size;
    ctx->allocated_size = new_allocated;
    
    return result;
}

static void* virtual_push_zero(void* context, uint64_t size) {
    void* result = virtual_push(context, size);
    if (result) {
        memset(result, 0, size);
    }
    return result;
}

static uint64_t virtual_allocated(void* context) {
    if (!context) return 0;
    
    VirtualContext* ctx = (VirtualContext*)context;
    return ctx->allocated_size;
}

static uint64_t virtual_capacity(void* context) {
    if (!context) return 0;
    
    VirtualContext* ctx = (VirtualContext*)context;
    return ctx->committed_size;
}

static SamrenaError virtual_reserve(void* context, uint64_t min_capacity) {
    if (!context) {
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    VirtualContext* ctx = (VirtualContext*)context;
    
    if (min_capacity > ctx->reserved_size) {
        return SAMRENA_ERROR_INVALID_PARAMETER;
    }
    
    if (ctx->committed_size >= min_capacity) {
        return SAMRENA_SUCCESS;
    }
    
    uint64_t needed = min_capacity - ctx->committed_size;
    uint64_t commit_size = ((needed + ctx->commit_granularity - 1) / 
                           ctx->commit_granularity) * ctx->commit_granularity;
    
    uint64_t new_committed = ctx->committed_size + commit_size;
    if (new_committed > ctx->reserved_size) {
        new_committed = ctx->reserved_size;
        commit_size = new_committed - ctx->committed_size;
    }
    
    if (commit_size > 0) {
        void* commit_addr = (uint8_t*)ctx->base_address + ctx->committed_size;
        if (!virtual_commit(commit_addr, commit_size)) {
            return SAMRENA_ERROR_OUT_OF_MEMORY;
        }
        ctx->committed_size = new_committed;
    }
    
    return SAMRENA_SUCCESS;
}

static void virtual_reset(void* context) {
    if (!context) return;
    
    VirtualContext* ctx = (VirtualContext*)context;
    ctx->allocated_size = 0;
}

static void virtual_dump_stats(void* context, FILE* out) {
    if (!context || !out) return;
    
    VirtualContext* ctx = (VirtualContext*)context;
    
    fprintf(out, "=== Virtual Adapter Statistics ===\n");
    fprintf(out, "Base address: %p\n", ctx->base_address);
    fprintf(out, "Reserved size: %lu bytes (%.2f MB)\n", 
            ctx->reserved_size, ctx->reserved_size / (1024.0 * 1024.0));
    fprintf(out, "Committed size: %lu bytes (%.2f MB)\n", 
            ctx->committed_size, ctx->committed_size / (1024.0 * 1024.0));
    fprintf(out, "Allocated size: %lu bytes (%.2f MB)\n", 
            ctx->allocated_size, ctx->allocated_size / (1024.0 * 1024.0));
    fprintf(out, "Page size: %lu bytes\n", ctx->page_size);
    fprintf(out, "Commit granularity: %lu bytes\n", ctx->commit_granularity);
    
    if (ctx->reserved_size > 0) {
        fprintf(out, "Virtual utilization: %.2f%%\n", 
                (double)ctx->committed_size / ctx->reserved_size * 100.0);
    }
    
    if (ctx->committed_size > 0) {
        fprintf(out, "Physical utilization: %.2f%%\n", 
                (double)ctx->allocated_size / ctx->committed_size * 100.0);
    }
    
    fprintf(out, "Allocation granularity: %lu bytes\n", get_allocation_granularity());
}

// Virtual adapter capabilities
static const SamrenaCapabilities virtual_capabilities = {
    .flags = SAMRENA_CAP_CONTIGUOUS_MEMORY |
             SAMRENA_CAP_ZERO_COPY_GROWTH |
             SAMRENA_CAP_RESET |
             SAMRENA_CAP_RESERVE |
             SAMRENA_CAP_MEMORY_STATS,
    .max_allocation_size = 0,  // Set dynamically based on reserved size
    .max_total_size = 0,       // Set dynamically based on platform
    .allocation_granularity = 1,
    .alignment_guarantee = 16,  // Better alignment than chained
    .allocation_overhead = 0.0,
};

static const SamrenaCapabilities* virtual_get_capabilities(void* context) {
    if (!context) {
        // Return static capabilities for strategy queries
        return &virtual_capabilities;
    }
    
    VirtualContext* ctx = (VirtualContext*)context;
    static SamrenaCapabilities caps;
    caps = virtual_capabilities;
    
    // Set dynamic values based on current state
    caps.max_allocation_size = ctx->reserved_size - ctx->allocated_size;
    caps.max_total_size = ctx->reserved_size;
    
    return &caps;
}

// Virtual adapter prefetch implementation
static void virtual_prefetch(void* context, uint64_t expected_size) {
    if (!context) return;
    
    VirtualContext* ctx = (VirtualContext*)context;
    
    // Ensure enough memory is committed
    uint64_t needed_committed = ctx->allocated_size + expected_size;
    if (needed_committed > ctx->committed_size) {
        uint64_t to_commit = needed_committed - ctx->committed_size;
        // Round up to commit granularity
        to_commit = ((to_commit + ctx->commit_granularity - 1) / 
                     ctx->commit_granularity) * ctx->commit_granularity;
        
        if (ctx->committed_size + to_commit <= ctx->reserved_size) {
            virtual_commit((uint8_t*)ctx->base_address + ctx->committed_size, to_commit);
            ctx->committed_size += to_commit;
        }
    }
    
    // Platform-specific prefetch hints
#ifdef _WIN32
    WIN32_MEMORY_RANGE_ENTRY entry = {
        .VirtualAddress = ctx->base_address,
        .NumberOfBytes = expected_size
    };
    PrefetchVirtualMemory(GetCurrentProcess(), 1, &entry, 0);
#elif defined(__linux__)
    madvise(ctx->base_address, expected_size, MADV_WILLNEED);
#endif
}

const SamrenaOps virtual_adapter_ops = {
    .name = "virtual",
    .create = virtual_create,
    .destroy = virtual_destroy,
    .push = virtual_push,
    .push_zero = virtual_push_zero,
    .allocated = virtual_allocated,
    .capacity = virtual_capacity,
    .reserve = virtual_reserve,
    .reset = virtual_reset,
    .get_capabilities = virtual_get_capabilities,
    .save_point = NULL,       // Not implemented yet
    .restore_point = NULL,    // Not implemented yet
    .prefetch = virtual_prefetch,
    .dump_stats = virtual_dump_stats
};