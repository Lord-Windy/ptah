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
#include "samrena_config.h"
#include "samrena_internal.h"
#include "adapters/chained_adapter.h"
#ifdef SAMRENA_ENABLE_VIRTUAL
#include "adapters/virtual_adapter.h"
#endif
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys/sysinfo.h>
#endif

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

// Adapter registry
typedef struct {
    SamrenaStrategy strategy;
    const SamrenaOps* ops;
    const char* name;
} AdapterEntry;

static const AdapterEntry adapters[] = {
    { SAMRENA_STRATEGY_CHAINED, &chained_adapter_ops, "chained" },
#ifdef SAMRENA_ENABLE_VIRTUAL
    { SAMRENA_STRATEGY_VIRTUAL, &virtual_adapter_ops, "virtual" },
#endif
    { 0, NULL, NULL }  // Sentinel
};

static const SamrenaOps* find_adapter(SamrenaStrategy strategy) {
    for (const AdapterEntry* entry = adapters; entry->ops; entry++) {
        if (entry->strategy == strategy) {
            return entry->ops;
        }
    }
    return NULL;
}

// Fallback rules structure
typedef struct {
    SamrenaStrategy from;
    SamrenaStrategy to;
    const char* reason;
} FallbackRule;

static const FallbackRule fallback_rules[] = {
    { SAMRENA_STRATEGY_VIRTUAL, SAMRENA_STRATEGY_CHAINED, 
      "Virtual memory not available on this platform" },
    { SAMRENA_STRATEGY_DEFAULT, SAMRENA_STRATEGY_CHAINED,
      "No optimal strategy available, using chained" },
    { 0, 0, NULL }
};

static void log_message(const SamrenaConfig* config, const char* format, ...) {
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

// Configuration validation
static SamrenaError validate_config(const SamrenaConfig* config) {
    if (!config) {
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    if (config->initial_pages == 0) {
        return SAMRENA_ERROR_INVALID_PARAMETER;
    }
    
    if (config->page_size != 0 && config->page_size < 4096) {
        return SAMRENA_ERROR_INVALID_PARAMETER;  // Minimum page size
    }
    
    if (config->strategy < 0 || config->strategy > SAMRENA_STRATEGY_VIRTUAL) {
        return SAMRENA_ERROR_INVALID_PARAMETER;
    }
    
    // Virtual memory specific validation
    if (config->strategy == SAMRENA_STRATEGY_VIRTUAL) {
        if (config->max_reserve != 0 && config->page_size != 0 && 
            config->max_reserve < config->page_size) {
            return SAMRENA_ERROR_INVALID_PARAMETER;
        }
    }
    
    return SAMRENA_SUCCESS;
}

static SamrenaStrategy apply_fallback(
    SamrenaStrategy requested,
    const SamrenaConfig* config,
    const char** reason
) {
    // Check if requested is available
    if (find_adapter(requested)) {
        return requested;
    }
    
    // Handle based on fallback mode
    if (config->fallback_mode == SAMRENA_FALLBACK_STRICT) {
        *reason = "Requested strategy not available";
        return SAMRENA_STRATEGY_DEFAULT;  // Signal error
    }
    
    // Find appropriate fallback
    for (const FallbackRule* rule = fallback_rules; rule->reason; rule++) {
        if (rule->from == requested && find_adapter(rule->to)) {
            *reason = rule->reason;
            return rule->to;
        }
    }
    
    // Last resort
    *reason = "No suitable adapter found";
    return SAMRENA_STRATEGY_CHAINED;
}

static SamrenaStrategy select_strategy(SamrenaStrategy requested) {
    switch (requested) {
    case SAMRENA_STRATEGY_DEFAULT:
#ifdef SAMRENA_ENABLE_VIRTUAL
        // Prefer virtual memory when available
        return SAMRENA_STRATEGY_VIRTUAL;
#else
        return SAMRENA_STRATEGY_CHAINED;
#endif
    
    default:
        return requested;
    }
}

// System resource detection for strategy selection
static void detect_system_resources(SamrenaAllocationHints* hints) {
#ifdef _WIN32
    MEMORYSTATUSEX status = { sizeof(status) };
    if (GlobalMemoryStatusEx(&status)) {
        uint64_t available = status.ullAvailPhys;
        
        // Set low memory mode if less than 1GB available
        if (available < 1024 * 1024 * 1024) {
            hints->low_memory_mode = true;
        }
        
        // Set memory limit to 75% of available
        if (hints->max_memory_limit == 0) {
            hints->max_memory_limit = available * 3 / 4;
        }
    }
#elif defined(__linux__)
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        uint64_t available = info.freeram * info.mem_unit;
        
        if (available < 1024 * 1024 * 1024) {
            hints->low_memory_mode = true;
        }
        
        if (hints->max_memory_limit == 0) {
            hints->max_memory_limit = available * 3 / 4;
        }
    }
#endif
}

// Strategy scoring system
typedef struct {
    SamrenaStrategy strategy;
    int score;
    const char* reason;
} StrategyScore;

static int score_strategy(
    SamrenaStrategy strategy,
    const SamrenaAllocationHints* hints
) {
    int score = 100;  // Base score
    
    switch (strategy) {
    case SAMRENA_STRATEGY_VIRTUAL:
        if (!samrena_strategy_available(strategy)) {
            return -1;  // Not available
        }
        
        // Excellent for contiguous memory
        if (hints->require_contiguous) {
            score += 50;
        }
        
        // Good for large allocations
        if (hints->expected_total_size > 100 * 1024 * 1024) {
            score += 30;
        }
        
        // Penalty for low memory mode
        if (hints->low_memory_mode) {
            score -= 20;  // Reserves address space
        }
        break;
        
    case SAMRENA_STRATEGY_CHAINED:
        if (!samrena_strategy_available(strategy)) {
            return -1;
        }
        
        // Good for incremental growth
        if (hints->expected_alloc_count > 1000) {
            score += 20;
        }
        
        // Excellent for low memory
        if (hints->low_memory_mode) {
            score += 40;
        }
        
        // Penalty for contiguous requirement
        if (hints->require_contiguous) {
            score -= 100;  // Cannot provide
        }
        
        // Good for frequent resets
        if (hints->frequent_reset) {
            score += 20;
        }
        break;
        
    case SAMRENA_STRATEGY_DEFAULT:
        // Default strategy should not be scored directly
        return -1;
    }
    
    return score;
}

static SamrenaStrategy select_best_strategy(
    const SamrenaAllocationHints* hints
) {
    StrategyScore scores[] = {
        { SAMRENA_STRATEGY_VIRTUAL, 0, NULL },
        { SAMRENA_STRATEGY_CHAINED, 0, NULL },
    };
    
    int best_score = -1;
    SamrenaStrategy best_strategy = SAMRENA_STRATEGY_CHAINED;
    
    for (size_t i = 0; i < sizeof(scores)/sizeof(scores[0]); i++) {
        scores[i].score = score_strategy(scores[i].strategy, hints);
        
        if (scores[i].score > best_score) {
            best_score = scores[i].score;
            best_strategy = scores[i].strategy;
        }
    }
    
    return best_strategy;
}

static const SamrenaOps* get_adapter_ops(SamrenaStrategy strategy) {
    SamrenaStrategy selected = select_strategy(strategy);
    return find_adapter(selected);
}

Samrena* samrena_create(const SamrenaConfig* config) {
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
        cfg.page_size = 64 * 1024;  // 64KB default page size
    }
    
    // Select strategy with fallback
    const char* fallback_reason = NULL;
    SamrenaStrategy selected = apply_fallback(
        cfg.strategy, &cfg, &fallback_reason
    );
    
    if (selected == SAMRENA_STRATEGY_DEFAULT) {
        // Fallback failed in strict mode
        samrena_set_error(SAMRENA_ERROR_UNSUPPORTED_STRATEGY);
        return NULL;
    }
    
    // Find adapter
    const SamrenaOps* ops = find_adapter(selected);
    if (!ops) {
        log_message(&cfg, "Internal error: adapter not found");
        samrena_set_error(SAMRENA_ERROR_UNSUPPORTED_STRATEGY);
        return NULL;
    }
    
    // Allocate arena structure
    Samrena* arena = calloc(1, sizeof(Samrena));
    if (!arena) {
        samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    arena->impl = calloc(1, sizeof(SamrenaImpl));
    if (!arena->impl) {
        free(arena);
        samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    // Initialize implementation
    arena->impl->ops = ops;
    arena->impl->strategy = selected;
    arena->impl->page_size = cfg.page_size;
    arena->impl->config = cfg;
    strncpy(arena->impl->adapter_name, samrena_strategy_name(selected), 
            sizeof(arena->impl->adapter_name) - 1);
    
    // Create adapter context
    err = ops->create(&arena->context, &cfg);
    if (err != SAMRENA_SUCCESS) {
        log_message(&cfg, "Failed to create %s adapter: error %d", 
                    samrena_strategy_name(selected), err);
        free(arena->impl);
        free(arena);
        samrena_set_error(err);
        return NULL;
    }
    
    // Log successful creation if fallback occurred
    if (cfg.log_callback && selected != cfg.strategy && fallback_reason) {
        log_message(&cfg, "Created %s arena (requested: %s, reason: %s)", 
                    samrena_strategy_name(selected), 
                    samrena_strategy_name(cfg.strategy),
                    fallback_reason);
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

// Capability query API implementation
bool samrena_strategy_available(SamrenaStrategy strategy) {
    return find_adapter(strategy) != NULL;
}

int samrena_available_strategies(SamrenaStrategy* strategies, int max_count) {
    int count = 0;
    for (const AdapterEntry* entry = adapters; 
         entry->ops && count < max_count; 
         entry++) {
        if (strategies) {
            strategies[count] = entry->strategy;
        }
        count++;
    }
    return count;
}

const char* samrena_strategy_name(SamrenaStrategy strategy) {
    for (const AdapterEntry* entry = adapters; entry->ops; entry++) {
        if (entry->strategy == strategy) {
            return entry->name;
        }
    }
    return "unknown";
}

// Performance hints for each adapter type
static const SamrenaPerformanceHints chained_hints = {
    .contiguous_memory = false,
    .zero_copy_growth = true,
    .constant_time_alloc = true,
    .max_single_alloc = UINT64_MAX
};

#ifdef SAMRENA_ENABLE_VIRTUAL
static const SamrenaPerformanceHints virtual_hints = {
    .contiguous_memory = true,
    .zero_copy_growth = true,
    .constant_time_alloc = true,
    .max_single_alloc = 0  // Set based on reserved size
};
#endif

SamrenaPerformanceHints samrena_get_performance_hints(Samrena* arena) {
    if (!arena || !arena->impl) {
        return (SamrenaPerformanceHints){0};
    }
    
    switch (arena->impl->config.strategy) {
        case SAMRENA_STRATEGY_CHAINED:
            return chained_hints;
#ifdef SAMRENA_ENABLE_VIRTUAL
        case SAMRENA_STRATEGY_VIRTUAL: {
            SamrenaPerformanceHints hints = virtual_hints;
            hints.max_single_alloc = arena->impl->config.max_reserve;
            return hints;
        }
#endif
        default:
            return chained_hints;
    }
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

// Factory and strategy selection API implementation
Samrena* samrena_create_with_hints(const SamrenaAllocationHints* hints) {
    SamrenaAllocationHints local_hints = hints ? *hints : (SamrenaAllocationHints){0};
    
    // Auto-detect system resources
    detect_system_resources(&local_hints);
    
    // Select best strategy
    SamrenaStrategy strategy = select_best_strategy(&local_hints);
    
    // Build configuration from hints
    SamrenaConfig config = samrena_default_config();
    config.strategy = strategy;
    
    // Configure based on hints
    if (local_hints.expected_total_size > 0) {
        uint64_t page_size = config.page_size ? config.page_size : 64 * 1024;
        config.initial_pages = (local_hints.expected_total_size + 
                               page_size - 1) / page_size;
    }
    
    if (strategy == SAMRENA_STRATEGY_VIRTUAL && 
        local_hints.expected_total_size > 0) {
        // Reserve 2x expected size for growth
        config.max_reserve = local_hints.expected_total_size * 2;
    }
    
    // Create arena with selected configuration
    return samrena_create(&config);
}

Samrena* samrena_create_default(void) {
    return samrena_create(NULL);  // Use all defaults
}

Samrena* samrena_create_for_size(uint64_t expected_size) {
    SamrenaAllocationHints hints = {
        .expected_total_size = expected_size,
    };
    return samrena_create_with_hints(&hints);
}

Samrena* samrena_create_high_performance(uint64_t initial_mb) {
    SamrenaConfig config = samrena_default_config();
    config.strategy = SAMRENA_STRATEGY_VIRTUAL;
    config.initial_pages = (initial_mb * 1024 * 1024) / (64 * 1024);
    config.max_reserve = initial_mb * 10 * 1024 * 1024;  // 10x headroom
    return samrena_create(&config);
}

Samrena* samrena_create_low_memory(void) {
    SamrenaConfig config = samrena_default_config();
    config.strategy = SAMRENA_STRATEGY_CHAINED;
    config.initial_pages = 1;
    config.growth_pages = 1;
    config.page_size = 4096;  // Small pages
    return samrena_create(&config);
}

Samrena* samrena_create_temp(void) {
    SamrenaAllocationHints hints = {
        .frequent_reset = true,
        .expected_total_size = 1024 * 1024,  // 1MB
    };
    return samrena_create_with_hints(&hints);
}

// Utility functions implementation
bool samrena_can_allocate(Samrena* arena, uint64_t size) {
    if (!arena || !arena->impl) return false;
    
    // For virtual memory, check against reserved space
    if (arena->impl->strategy == SAMRENA_STRATEGY_VIRTUAL) {
        uint64_t used = samrena_allocated(arena);
        uint64_t capacity = samrena_capacity(arena);
        return (used + size) <= capacity;
    }
    
    // For chained, always true (can grow)
    return true;
}

void* samrena_push_aligned(Samrena* arena, uint64_t size, uint64_t alignment) {
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
    void* temp_ptr = samrena_push(arena, 1);
    if (!temp_ptr) {
        return NULL;
    }
    
    uintptr_t current_addr = (uintptr_t)temp_ptr;
    uintptr_t aligned_addr = (current_addr + alignment - 1) & ~(alignment - 1);
    uint64_t padding = aligned_addr - current_addr;
    
    // Allocate additional padding if needed (we already allocated 1 byte)
    if (padding > 1) {
        void* padding_ptr = samrena_push(arena, padding - 1);
        if (!padding_ptr) {
            return NULL;
        }
    }
    
    // Now allocate the actual data (the first byte of the aligned block is our temp byte)
    if (size > 1) {
        void* data_ptr = samrena_push(arena, size - 1);
        if (!data_ptr) {
            return NULL;
        }
    }
    
    // Return the aligned address
    return (void*)aligned_addr;
}

bool samrena_reset_if_supported(Samrena* arena) {
    if (!arena || !arena->impl || !arena->impl->ops->reset) {
        return false;
    }
    arena->impl->ops->reset(arena->context);
    return true;
}

void samrena_get_info(Samrena* arena, SamrenaInfo* info) {
    if (!arena || !arena->impl || !info) return;
    
    info->adapter_name = arena->impl->adapter_name;
    info->strategy = arena->impl->strategy;
    info->allocated = samrena_allocated(arena);
    info->capacity = samrena_capacity(arena);
    info->page_size = arena->impl->page_size;
    
    // Strategy-specific info
    switch (arena->impl->strategy) {
    case SAMRENA_STRATEGY_VIRTUAL:
        info->can_grow = false;  // Fixed reservation
        info->is_contiguous = true;
        break;
    case SAMRENA_STRATEGY_CHAINED:
        info->can_grow = true;
        info->is_contiguous = false;
        break;
    default:
        info->can_grow = false;
        info->is_contiguous = false;
    }
}