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
        case SAMRENA_ERROR_INVALID_PARAMETER:
            return "Invalid parameter error";
        case SAMRENA_ERROR_UNSUPPORTED_STRATEGY:
            return "Unsupported strategy error";
        case SAMRENA_ERROR_UNSUPPORTED_OPERATION:
            return "Unsupported operation error";
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




Samrena* samrena_create_default(void) {
    return samrena_create(NULL);  // Use all defaults
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

// ============================================================================
// Phase 5: Enhanced Features Implementation
// ============================================================================

// Capability Query API Implementation
SamrenaCapabilities samrena_get_capabilities(Samrena* arena) {
    if (!arena || !arena->impl || !arena->impl->ops->get_capabilities) {
        return (SamrenaCapabilities){0};
    }
    
    const SamrenaCapabilities* base_caps = arena->impl->ops->get_capabilities(arena->context);
    if (!base_caps) {
        return (SamrenaCapabilities){0};
    }
    
    SamrenaCapabilities caps = *base_caps;
    
    // Adjust dynamic values based on current state
    if (arena->impl->strategy == SAMRENA_STRATEGY_VIRTUAL) {
        // For virtual adapter, update actual limits based on reservation
        // This would be filled in by the virtual adapter implementation
        uint64_t current_used = samrena_allocated(arena);
        uint64_t total_capacity = samrena_capacity(arena);
        caps.max_allocation_size = total_capacity - current_used;
    }
    
    return caps;
}

bool samrena_has_capability(Samrena* arena, SamrenaCapabilityFlags cap) {
    SamrenaCapabilities caps = samrena_get_capabilities(arena);
    return (caps.flags & cap) != 0;
}

SamrenaCapabilities samrena_strategy_capabilities(SamrenaStrategy strategy) {
    const SamrenaOps* ops = find_adapter(strategy);
    if (!ops || !ops->get_capabilities) {
        return (SamrenaCapabilities){0};
    }
    
    // For strategy capabilities, we can't provide context-specific info
    // Return the static capabilities with maximum values
    const SamrenaCapabilities* base_caps = ops->get_capabilities(NULL);
    return base_caps ? *base_caps : (SamrenaCapabilities){0};
}

// Memory Reservation API Implementation
SamrenaError samrena_reserve(Samrena* arena, uint64_t size) {
    if (!arena || !arena->impl) {
        return SAMRENA_ERROR_INVALID_PARAMETER;
    }
    
    if (arena->impl->ops->reserve) {
        return arena->impl->ops->reserve(arena->context, size);
    }
    
    // If reserve not supported, return OK (it's a hint)
    return SAMRENA_SUCCESS;
}

SamrenaError samrena_reserve_with_growth(
    Samrena* arena, 
    uint64_t immediate_size,
    uint64_t expected_total
) {
    if (!arena || !arena->impl) {
        return SAMRENA_ERROR_INVALID_PARAMETER;
    }
    
    // Store growth hint for future allocations
    arena->impl->growth_hint = expected_total;
    
    // Reserve immediate size plus some headroom
    uint64_t reserve_size = immediate_size * 2;
    if (reserve_size < expected_total / 4) {
        reserve_size = expected_total / 4;  // Reserve 25% of expected
    }
    
    return samrena_reserve(arena, reserve_size);
}

// Growth Policies Implementation
typedef struct {
    // Allocation history for adaptive growth
    uint64_t* allocation_sizes;
    size_t history_size;
    size_t history_capacity;
    size_t history_index;
    
    // Statistics
    uint64_t total_growths;
    uint64_t total_wasted;
    uint64_t allocation_count;
} GrowthState;


static uint64_t calculate_growth(
    const SamrenaGrowthConfig* config,
    GrowthState* state,
    uint64_t current_size,
    uint64_t requested_size
) {
    uint64_t min_growth = requested_size - current_size;
    uint64_t new_size = current_size;
    
    switch (config->policy) {
    case SAMRENA_GROWTH_LINEAR:
        new_size += config->params.linear.increment;
        break;
        
    case SAMRENA_GROWTH_EXPONENTIAL:
        new_size = (uint64_t)(current_size * config->params.exponential.factor);
        break;
        
    }
    
    // Ensure minimum growth
    if (new_size < current_size + min_growth) {
        new_size = current_size + min_growth;
    }
    
    return new_size - current_size;
}


// Preset growth configurations
const SamrenaGrowthConfig SAMRENA_GROWTH_CONSERVATIVE = {
    .policy = SAMRENA_GROWTH_LINEAR,
    .params.linear = { .increment = 65536 }  // 64KB at a time
};

const SamrenaGrowthConfig SAMRENA_GROWTH_BALANCED = {
    .policy = SAMRENA_GROWTH_EXPONENTIAL,
    .params.exponential = { .factor = 1.5 }
};


