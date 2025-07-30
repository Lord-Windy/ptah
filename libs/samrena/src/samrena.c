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
    if (!config) {
        samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
        return NULL;
    }
    
    SamrenaConfig cfg = *config;
    
    const char* fallback_reason = NULL;
    SamrenaStrategy selected = apply_fallback(
        cfg.strategy, &cfg, &fallback_reason
    );
    
    if (selected != cfg.strategy && fallback_reason) {
        if (cfg.fallback_mode == SAMRENA_FALLBACK_STRICT) {
            log_message(&cfg, "ERROR: %s", fallback_reason);
            samrena_set_error(SAMRENA_ERROR_UNSUPPORTED_STRATEGY);
            return NULL;
        } else if (cfg.fallback_mode == SAMRENA_FALLBACK_WARN) {
            log_message(&cfg, "WARNING: %s", fallback_reason);
        }
    }
    
    const SamrenaOps* ops = find_adapter(selected);
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
    cfg.strategy = selected;  // Update config with selected strategy
    impl->config = cfg;
    arena->impl = impl;
    arena->context = NULL;
    
    // Create the adapter context
    SamrenaError error = ops->create(&arena->context, &cfg);
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