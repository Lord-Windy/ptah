# Task: Implement Convenience Functions

## Overview
Create convenience functions that maintain backward compatibility and provide simple interfaces for common use cases.

## Requirements
- Maintain existing API compatibility
- Provide simple one-line arena creation
- Common configuration presets
- Helper functions for typical patterns

## Implementation Details

### 1. Backward Compatibility Functions
```c
// In include/samrena.h

// Original simple API - maintained for compatibility
Samrena* samrena_allocate(uint64_t initial_pages) {
    SamrenaConfig config = samrena_default_config();
    config.initial_pages = initial_pages;
    config.strategy = SAMRENA_STRATEGY_CHAINED;  // Keep original behavior
    return samrena_create(&config);
}

// Convenience wrappers matching original API
void* samrena_push(Samrena* arena, uint64_t size) {
    if (!arena || !arena->impl || !arena->impl->ops) {
        return NULL;
    }
    return arena->impl->ops->push(arena->context, size);
}

void* samrena_push_zero(Samrena* arena, uint64_t size) {
    if (!arena || !arena->impl || !arena->impl->ops) {
        return NULL;
    }
    return arena->impl->ops->push_zero(arena->context, size);
}

uint64_t samrena_allocated(Samrena* arena) {
    if (!arena || !arena->impl || !arena->impl->ops) {
        return 0;
    }
    return arena->impl->ops->allocated(arena->context);
}
```

### 2. Configuration Presets
```c
// Common configuration builders
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
    config.initial_pages = (initial_mb * 1024 * 1024) / config.page_size;
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

// For temporary allocations
Samrena* samrena_create_temp(void) {
    SamrenaAllocationHints hints = {
        .frequent_reset = true,
        .expected_total_size = 1024 * 1024,  // 1MB
    };
    return samrena_create_with_hints(&hints);
}
```

### 3. Typed Allocation Helpers
```c
// Type-safe allocation macros
#define SAMRENA_PUSH_TYPE(arena, type) \
    ((type*)samrena_push((arena), sizeof(type)))

#define SAMRENA_PUSH_ARRAY(arena, type, count) \
    ((type*)samrena_push((arena), sizeof(type) * (count)))

#define SAMRENA_PUSH_TYPE_ZERO(arena, type) \
    ((type*)samrena_push_zero((arena), sizeof(type)))

#define SAMRENA_PUSH_ARRAY_ZERO(arena, type, count) \
    ((type*)samrena_push_zero((arena), sizeof(type) * (count)))

// Aligned allocation helpers
void* samrena_push_aligned(Samrena* arena, uint64_t size, uint64_t alignment) {
    // Calculate padding needed
    uint64_t current = samrena_allocated(arena);
    uint64_t padding = (alignment - (current % alignment)) % alignment;
    
    // Allocate padding
    if (padding > 0) {
        samrena_push(arena, padding);
    }
    
    // Allocate actual data
    return samrena_push(arena, size);
}

#define SAMRENA_PUSH_ALIGNED_TYPE(arena, type, alignment) \
    ((type*)samrena_push_aligned((arena), sizeof(type), (alignment)))
```

### 4. Utility Functions
```c
// Check if arena has enough space
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

// Get arena information
typedef struct {
    const char* adapter_name;
    SamrenaStrategy strategy;
    uint64_t allocated;
    uint64_t capacity;
    uint64_t page_size;
    bool can_grow;
    bool is_contiguous;
} SamrenaInfo;

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

// Reset helper with verification
bool samrena_reset_if_supported(Samrena* arena) {
    if (!arena || !arena->impl || !arena->impl->ops->reset) {
        return false;
    }
    arena->impl->ops->reset(arena->context);
    return true;
}
```

## Location
- `libs/samrena/include/samrena.h` - All convenience functions
- `libs/samrena/src/samrena.c` - Implementation of complex helpers

## Dependencies
- Task 10: Factory function
- Task 11: Strategy selection

## Verification
- [ ] Original API works unchanged
- [ ] All presets create valid arenas
- [ ] Type-safe macros compile correctly
- [ ] Aligned allocation works
- [ ] Info function returns accurate data

## Notes
- Keep convenience functions in header when possible
- Document which functions are thread-safe
- Add examples to documentation