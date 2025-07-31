# Task: Implement Adapter Capabilities

## Overview
Create a capability system that allows querying what features each adapter supports, enabling runtime feature detection and conditional code paths.

## Requirements
- Query supported operations per adapter
- Performance characteristics
- Limitation discovery
- Feature flags

## Implementation Details

### 1. Capability Flags
```c
// In include/samrena.h
typedef enum {
    SAMRENA_CAP_CONTIGUOUS_MEMORY  = 1 << 0,  // All allocations in one block
    SAMRENA_CAP_ZERO_COPY_GROWTH   = 1 << 1,  // Can grow without moving data
    SAMRENA_CAP_RESET              = 1 << 2,  // Supports reset operation
    SAMRENA_CAP_RESERVE            = 1 << 3,  // Supports reserve operation
    SAMRENA_CAP_MEMORY_STATS       = 1 << 4,  // Tracks statistics
    SAMRENA_CAP_LARGE_ALLOCATIONS  = 1 << 5,  // Can handle >2GB allocations
    SAMRENA_CAP_SAVE_RESTORE       = 1 << 6,  // Supports save/restore points
    SAMRENA_CAP_THREAD_SAFE        = 1 << 7,  // Thread-safe operations
} SamrenaCapabilityFlags;

typedef struct {
    uint32_t flags;                    // Capability flags
    uint64_t max_allocation_size;      // Largest single allocation
    uint64_t max_total_size;          // Maximum total arena size
    uint64_t allocation_granularity;   // Minimum allocation unit
    uint64_t alignment_guarantee;      // Guaranteed alignment
    double allocation_overhead;        // Overhead per allocation (0.0-1.0)
} SamrenaCapabilities;
```

### 2. Adapter Capability Definitions
```c
// In src/adapter_chained.c
static const SamrenaCapabilities chained_capabilities = {
    .flags = SAMRENA_CAP_ZERO_COPY_GROWTH | 
             SAMRENA_CAP_RESET |
             SAMRENA_CAP_LARGE_ALLOCATIONS,
    .max_allocation_size = UINT64_MAX,
    .max_total_size = UINT64_MAX,
    .allocation_granularity = 1,
    .alignment_guarantee = sizeof(void*),
    .allocation_overhead = 0.0,  // No per-allocation overhead
};

// In src/adapter_virtual.c
static const SamrenaCapabilities virtual_capabilities = {
    .flags = SAMRENA_CAP_CONTIGUOUS_MEMORY |
             SAMRENA_CAP_ZERO_COPY_GROWTH |
             SAMRENA_CAP_RESET |
             SAMRENA_CAP_RESERVE |
             SAMRENA_CAP_SAVE_RESTORE,
    .max_allocation_size = 0,  // Set based on reserved size
    .max_total_size = 0,       // Set based on platform
    .allocation_granularity = 1,
    .alignment_guarantee = 16,  // Better alignment
    .allocation_overhead = 0.0,
};
```

### 3. Extended Operations Interface
```c
// In src/samrena_internal.h
typedef struct {
    // ... existing operations ...
    
    // Capability query
    const SamrenaCapabilities* (*get_capabilities)(void* context);
    
    // Optional advanced operations
    SamrenaError (*save_point)(void* context, void** savepoint);
    SamrenaError (*restore_point)(void* context, void* savepoint);
    
    // Performance hints
    void (*prefetch)(void* context, uint64_t expected_size);
} SamrenaOps;
```

### 4. Query Functions
```c
// In include/samrena.h
// Get capabilities of current arena
SamrenaCapabilities samrena_get_capabilities(Samrena* arena);

// Check specific capability
bool samrena_has_capability(Samrena* arena, SamrenaCapabilityFlags cap);

// Get capabilities for a strategy (without creating arena)
SamrenaCapabilities samrena_strategy_capabilities(SamrenaStrategy strategy);

// Implementation
SamrenaCapabilities samrena_get_capabilities(Samrena* arena) {
    if (!arena || !arena->impl || !arena->impl->ops->get_capabilities) {
        return (SamrenaCapabilities){0};
    }
    
    SamrenaCapabilities caps = *arena->impl->ops->get_capabilities(arena->context);
    
    // Adjust dynamic values
    if (arena->impl->strategy == SAMRENA_STRATEGY_VIRTUAL) {
        // Set actual limits based on reservation
        VirtualContext* ctx = (VirtualContext*)arena->context;
        caps.max_allocation_size = ctx->vm->reserved - ctx->used;
        caps.max_total_size = ctx->vm->reserved;
    }
    
    return caps;
}

bool samrena_has_capability(Samrena* arena, SamrenaCapabilityFlags cap) {
    SamrenaCapabilities caps = samrena_get_capabilities(arena);
    return (caps.flags & cap) != 0;
}
```

### 5. Capability-Based Code Paths
```c
// Example: Optimized array allocation
void* samrena_push_array_optimal(Samrena* arena, size_t elem_size, size_t count) {
    uint64_t total_size = elem_size * count;
    
    // Check for save/restore capability for large arrays
    if (total_size > 1024 * 1024 && 
        samrena_has_capability(arena, SAMRENA_CAP_SAVE_RESTORE)) {
        
        // Save point before large allocation
        void* savepoint = NULL;
        if (arena->impl->ops->save_point) {
            arena->impl->ops->save_point(arena->context, &savepoint);
        }
        
        void* result = samrena_push(arena, total_size);
        
        if (!result && savepoint) {
            // Allocation failed, restore
            arena->impl->ops->restore_point(arena->context, savepoint);
        }
        
        return result;
    }
    
    // Standard allocation
    return samrena_push(arena, total_size);
}

// Example: Conditional reset usage
void process_batch(Samrena* arena, void* data) {
    if (samrena_has_capability(arena, SAMRENA_CAP_RESET)) {
        // Can use fast reset between batches
        process_with_reset(arena, data);
    } else {
        // Must track allocations manually
        process_with_tracking(arena, data);
    }
}
```

## Location
- `libs/samrena/include/samrena.h` - Capability API
- `libs/samrena/src/samrena_internal.h` - Extended operations
- `libs/samrena/src/adapter_*.c` - Per-adapter capabilities

## Dependencies
- Tasks 04-06: Adapters implemented

## Verification
- [ ] All adapters report correct capabilities
- [ ] Dynamic values updated properly
- [ ] Capability queries are fast
- [ ] Feature detection works correctly
- [ ] Examples demonstrate usage

## Notes
- Keep capability checks lightweight
- Document which capabilities are guaranteed vs best-effort
- Consider capability versioning for future extensions